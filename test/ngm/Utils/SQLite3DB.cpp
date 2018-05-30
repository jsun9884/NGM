#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SQLite3DB.h"
#include "SqlDeviceMap.h"
#include "HardwareManager.h"


SQLite3DB::SQLite3DB(std::string dbFile, std::string sqlCreateScrFile):
    m_dbFile(dbFile), m_dbCreateScrFile(sqlCreateScrFile),bRunning(false),m_id(0)
{
    g_SqlDbFile = dbFile;
}

bool SQLite3DB::exec(std::string sql, SQLiteCallback callback)
{
    sqlite3 *db;
    int ret;
    bool res = false;
    char *zErrMsg = 0;

    log::info() << "SQL exec: " << sql;

    ret = sqlite3_open_v2(m_dbFile.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);

    if (ret != SQLITE_OK) {
        log::warn() << "Can't open database " << m_dbFile << " trying to create...";

#if 0
        ret = sqlite3_open_v2(m_dbFile.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        if (ret != SQLITE_OK) {
            log::error() << "Can't open & create database: " << m_dbFile;
            return false;
        }

        if (!createDB(db)) {
            sqlite3_close(db);
            return false;
        }
#else
        createDBaseFromCopy();

        ret = sqlite3_open_v2(m_dbFile.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);

        if (ret != SQLITE_OK) {
            log::warn() << "Can't open created database " << m_dbFile;
            return false;
        }

        m_id = getTableMaxId("native_registers_data") + 1;

#endif

        log::info() << "New database successfully created...";
    }

    m_execCB = callback;
    ret = sqlite3_exec(db, sql.c_str(), execCallback, reinterpret_cast<void *>(this), &zErrMsg);
    if ( ret != SQLITE_OK ) {
        log::error() << "SQL error " << ret << ": " << zErrMsg;
        sqlite3_free(zErrMsg);

#if 0
        g_bSqlNeedReset = true;
        bRunning = false;
#else
        sqlite3_close(db);
        createDBaseFromCopy();
        return res;
#endif
    }
    else {
        res = true;
    }

    sqlite3_close(db);

    //log::info() << "sql exec done...";
    return res;
}

SqlTablePtr SQLite3DB::addTable(std::string tblName)
{
    auto it = m_tables.find(tblName);

    if (it != m_tables.end()) {
        return nullptr;
    }

    SqlTablePtr table = std::make_shared<SqlTable>(tblName);
    m_tables.insert({tblName, table});

    return table;
}

SqlTablePtr SQLite3DB::getTable(std::string tblName)
{
    auto it = m_tables.find(tblName);

    if (it == m_tables.end()) {
        return nullptr;
    }

    return (*it).second;
}

std::string SQLite3DB::getInsertSqlScript()
{
    std::stringstream os;

    uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto it = m_tables.begin();
    auto it_end = m_tables.end();
    while( it != it_end ) {
        std::string rows, vals;
        bool bChanged = false;
        (*it).second->getRowsValues(rows, vals, bChanged);
        os << "INSERT INTO \"" << (*it).second->getName() << "\" (id,qualifier,timestamp,interval,associate," << rows << ") VALUES (";
        os << ++m_id << ",4," << ts << ",1000,\'" << (bChanged ? "BASELINE" : "PRESENT") << "\'," << vals <<");\n";
        ++it;
    }
    //os << "COMMIT;";

    return os.str();
}

void SQLite3DB::readMap(HardwareManager *hwMgr)
{
    ISupportConfigurationPtr dev;
    HRParameterPtr param;

    log::info() << "<<<< Reading SQL Device Map... >>>>";
    for (int i = 0; g_sqlDevMap[i].device != nullptr; ++i) {
        try {
            dev = hwMgr->getDevice(g_sqlDevMap[i].device);
            param = dev->getHRParameter(g_sqlDevMap[i].param);
        }
        catch (Exception &e) {
            log::info() << "Ex: " << e.what();
            continue;
        }

        SqlTablePtr tab = getTable(g_sqlDevMap[i].table);
        if (!tab) {
            tab = addTable(g_sqlDevMap[i].table);
            log::info() << "Create table: *** " << tab->getName() << " ***";
        }

        SqlRowPtr row = tab->addRow(g_sqlDevMap[i].row, param->getType(), dev);
        param->setSqlRow(row);

        log::info() << "Create row: " << tab->getName() << "." << row->getName()
                    << " <-> " << dev->getName() << "." << param->getName();
    }
    log::info() << "<<<< SQL Device Map readed... >>>>";
}

void SQLite3DB::start()
{
    if (!bRunning && !m_thread) {
        bRunning = true;
        m_thread = std::make_shared<std::thread>(&SQLite3DB::threadFunc, this);
    }
}

void SQLite3DB::stop()
{
    if (bRunning) {
        bRunning = false;
        m_thread->join();
    }
}

int SQLite3DB::execCallback(void *data, int argc, char **argv, char **azColName)
{
    SQLite3DB *o = reinterpret_cast<SQLite3DB *>(data);

    log::info() << "sql callback:";
    {
        for (int i = 0; i < argc; ++i) {
            log::info() << azColName[i] << " = " << argv[i];
        }
    }
    o->m_execCB(argc, argv, azColName);

    return SQLITE_OK;
}

bool SQLite3DB::createDB(sqlite3 *db)
{
    int ret;
    char *zErrMsg = 0;
    std::shared_ptr<StrBuffer> buf = std::make_shared<StrBuffer>(SQL_BUFFER_SIZE);

    FILE *pFile = fopen(m_dbCreateScrFile.c_str(), "r");
    if (!pFile) {
        log::error() << "Can't open sql script: " << m_dbCreateScrFile;
        return false;
    }

    size_t size = fread(buf->str, 1, SQL_BUFFER_SIZE, pFile);
    fclose(pFile);

    if (size <= 0) {
        log::error() << "Can't read sql script file: " << m_dbCreateScrFile;
        return false;
    }

    buf->str[size] = 0;

    m_execCB = [](int, char **, char **)->void{};

    ret = sqlite3_exec(db, buf->str, &execCallback, this, &zErrMsg);
    if (ret != SQLITE_OK) {
        log::error() << "createDB SQL error " << ret << ": " << zErrMsg;
        sqlite3_free(zErrMsg);
        return false;
    }

    log::info() << "SQL Script " << m_dbCreateScrFile << " executed...";

    return true;
}

void SQLite3DB::createDBaseFromCopy()
{
    std::string cmd = "cp -Rf \'" + m_dbFile + ".empty\' \'" + m_dbFile + "\'";

    log::info() << "exec: " << cmd;
    ::system(cmd.c_str());
}

void SQLite3DB::deleteFirstLinesFromTable(std::string table, int num)
{
    std::stringstream ss;
    ss << "delete from \'" << table << "\' where id in (select id from \'"
       << table << "\' limit " << num << ");";

    exec(ss.str());
}

uint32_t SQLite3DB::getDatabaseSize()
{
    struct stat sb;
    if (stat(m_dbFile.c_str(), &sb) == -1) {
        return 0;
    }

    return sb.st_size;
}

uint32_t SQLite3DB::getTableMaxId(std::string table)
{
    uint32_t out;
    std::stringstream ss;
    ss << "select max(id) from \'" << table << "\';";

    exec(ss.str(), [&out](int count, char **values, char **colNames){
        if (count > 0) {
            out = std::stol(values[0], 0, 0);
        }
    });

    //log::info() << "Max ID of \'" << table << "\': " << out;
    return out;
}

uint32_t SQLite3DB::getTableLinesCount(std::string table)
{
    uint32_t out;
    std::stringstream ss;
    ss << "select count(timestamp) from \'" << table << "\';";

    exec(ss.str(), [&out](int count, char **values, char **colNames){
        if (count > 0) {
            out = std::stol(values[0], 0, 0);
        }
    });

    //log::info() << "Lines counf of \'" << table << "\': " << out;
    return out;
}

void SQLite3DB::threadFunc()
{
    auto dur = std::chrono::seconds(1);
    auto nxt = std::chrono::system_clock::now();
    std::string sql;
    uint32_t db_size;

    log::info() << "Sql Loop started...";

    m_id = getTableMaxId("native_registers_data") + 1;

    while (bRunning) {

        db_size = getTableLinesCount("native_registers_data"); //getDatabaseSize();
        if (db_size < MAX_DB_SIZE) {

            sql = getInsertSqlScript();
            if (!exec(sql) ) {
                exec(sql);
            }
        }
        else {
            log::warn() << "Database is full...";
        }

        nxt += dur;
        std::this_thread::sleep_until(nxt);
    }

    log::info() << "Sql Loop stopped...";
}

StrBuffer::StrBuffer(size_t size)
{
    str = new char[size];
}

StrBuffer::~StrBuffer()
{
    delete [] str;
}

SqlTable::SqlTable(std::string name):
    m_name(name)
{
}

std::string SqlTable::getName() const
{
    return m_name;
}

SqlRowPtr SqlTable::addRow(std::string rowName, ValueType type, std::shared_ptr<ISupportConfiguration> dev)
{
    auto it = m_rows.find(rowName);

    if (it != m_rows.end()) {
        return nullptr;
    }

    SqlRowPtr row = std::make_shared<SqlRow>(rowName, type, dev);
    m_rows.insert({rowName, row});

    return row;
}

SqlRowPtr SqlTable::getRow(std::string rowName)
{
    auto it = m_rows.find(rowName);

    if (it == m_rows.end()) {
        return nullptr;
    }

    return (*it).second;
}

void SqlTable::getRowsValues(std::string &rows, std::string &vals, bool &bStatusChanged)
{
    std::stringstream os_rows, os_vals;
    auto it = m_rows.begin();
    auto it_end = m_rows.end();
    bool bHave = false;

    while(it != it_end) {
        if ((*it).second->isSensorStateChanged() && !bStatusChanged) {
            bStatusChanged = true;
        }

        if ((*it).second->getSensorState() == SensorState::Active) {
            if (bHave) {
                os_rows << ",";
                os_vals << ",";
            }
            else {
                bHave = true;
            }
            os_rows << (*it).second->getName();
            os_vals << (*it).second->getValue();
        }
        ++it;
    }
    rows = os_rows.str();
    vals = os_vals.str();
}

std::string SqlTable::getRows()
{
    std::stringstream os;
    auto it = m_rows.begin();
    auto it_end = m_rows.end();
    bool bHave = false;
    while(it != it_end) {
        if (bHave) {
            os << ",";
        }
        else {
            bHave = true;
        }
        os << (*it).second->getName();
        ++it;
    }
    return os.str();
}

std::string SqlTable::getValues()
{
    std::stringstream os;
    auto it = m_rows.begin();
    auto it_end = m_rows.end();
    bool bHave = false;
    while(it != it_end) {
        if (bHave) {
            os << ",";
        }
        else {
            bHave = true;
        }
        os << (*it).second->getValue();
        ++it;
    }
    return os.str();
}

SqlRow::SqlRow(std::string name, ValueType type, std::shared_ptr<ISupportConfiguration> dev):
    m_name(name),
    m_value(type),
    m_dev(dev)
{
}

std::string SqlRow::getName() const
{
    return m_name;
}

SensorState SqlRow::getSensorState()
{
    return m_dev->state();
}

bool SqlRow::isSensorStateChanged()
{
    return m_dev->popIsStateChanged();
}

std::string SqlRow::getValue()
{
    std::stringstream os;

    m_mutex.lock();
    AnyValue value(m_value);
    m_mutex.unlock();

    os << value;
    return os.str();
}
