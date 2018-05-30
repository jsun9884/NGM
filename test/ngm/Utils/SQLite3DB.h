#ifndef SQLITE3DB_H
#define SQLITE3DB_H

#include <functional>
#include <unordered_map>
#include <mutex>

#include "sqlite3.h"
#include "Log.h"
#include "AnyValue.h"
#include "SensorState.h"

#define MAX_DB_SIZE 100000
#define NUM_LINES_TO_DELETE 10
#define DATABASE_FILENAME "/home/linaro/c1222node/db/c12ngm-data-state-common.sqlite3"

class ISupportConfiguration;
class HardwareManager;
class SqlRow
{
public:
    SqlRow(std::string name, ValueType type, std::shared_ptr<ISupportConfiguration> dev);
    std::string getName() const;
    SensorState getSensorState();
    bool isSensorStateChanged();
    std::string getValue();

    template<class T>
    void setValue(const T &value) {
        std::lock_guard<std::mutex> l(m_mutex);
        m_value = value;
    }

private:
    std::string m_name;
    AnyValue m_value;
    std::shared_ptr<ISupportConfiguration> m_dev;
    std::mutex m_mutex;
};
typedef std::shared_ptr<SqlRow> SqlRowPtr;

class SqlTable
{
public:
    SqlTable(std::string name);
    std::string getName() const;
    SqlRowPtr addRow(std::string rowName, ValueType type, std::shared_ptr<ISupportConfiguration> dev);
    SqlRowPtr getRow(std::string rowName);
    void getRowsValues(std::string &rows, std::string &vals, bool &bStatusChanged);
    std::string getRows();
    std::string getValues();

private:
    std::string m_name;
    std::unordered_map<std::string, SqlRowPtr> m_rows;
};
typedef std::shared_ptr<SqlTable> SqlTablePtr;

using SQLiteCallback = std::function<void(int, char**, char **)>;

struct StrBuffer
{
    char *str;
    StrBuffer(size_t size);
    ~StrBuffer();
};

#define SQL_BUFFER_SIZE 256*1024

class SQLite3DB
{
    LOG_MODULE(SQLite3, g_sqlite3_log);

public:
    SQLite3DB(std::string dbFile, std::string sqlCreateScrFile);
    bool exec(std::string sql, SQLiteCallback callback = [](int, char**, char**)->void{});

    SqlTablePtr addTable(std::string tblName);
    SqlTablePtr getTable(std::string tblName);
    std::string getInsertSqlScript();

    void readMap(HardwareManager* hwMgr);

    void start();
    void stop();

private:
    static int execCallback(void *data, int argc, char **argv, char **azColName);
    bool createDB(sqlite3 *db);
    void createDBaseFromCopy();
    void deleteFirstLinesFromTable(std::string table, int num);
    uint32_t getDatabaseSize();
    uint32_t getTableMaxId(std::string table);
    uint32_t getTableLinesCount(std::string table);
    void threadFunc();

private:
    std::string m_dbFile;
    std::string m_dbCreateScrFile;
    SQLiteCallback m_execCB;
    bool bRunning;
    std::shared_ptr<std::thread> m_thread;
    int m_id;

private:
    std::unordered_map<std::string, SqlTablePtr> m_tables;
};

#endif // SQLITE3DB_H
