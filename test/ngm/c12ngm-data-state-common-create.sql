-- SQL Schema for C12.22 Node Interface to Generic SQLite 3
-- Register Set.
-- Original Design Model: C12.22 Node Demo
-- Date: 2017-03-23
-- Copyright Future DOS R&D Inc.
-- The licensee may modify this copy or copies of the database schema any portion of it,
-- thus forming a work based on this schema, and copy and distribute such modifications
-- or work under without restriction, provided that the schema caries modification history
-- and this introductory statement is preserved in the schema.
--
-- History
-- 2017-03-23    Written core example using the FDOS C12.22 Node Demo IoT Linux platform.       AM
-- 2018-02-15    Initial adaptation to NGM register set.                                        AM
-- 2018-03-26    Revised to include all registers supported by the NGM.                         AM

--
/*
 * Use-case for this database in the context of the NGM (See Figure 1a of V12.19 Implementation Proposal)
 *
 * The meter reads a collection of sensors (Sensors and measurements module) and prepares them
 * (computing and recording) for consumption by a C12.19 Table Processor by updating this database
 * continuously (Data & state common storage) as follows:
 * 
 * 1) power up and/or device reset
 *    1.1 Recover network time and absolute totals of applicable registers.
 *    1.2 Update/create this database with recovered and initialized data.
 * 2) Publish availability of data into table 'native_registers_data' (so that the C12.19 Process may use it).
 * 3) Update native_registers_data SQL table periodically at the desired application rate.
 * 4) Ensure that totalizers are maintained to survive power failure in a manner that a published/read
 *    value does not role back.
 * 5) Repeat steps 2-5 until shutdown
 * 6) Save database on shutdown then shutdown
 * 
 * Notes:
 * 1) When the database is saved to non persistent storage then the 'BASELINE' records of table 'native_registers_data'
 *    should be updated.
 * 2) The 'PRESENT' records of table 'native_registers_data' should be inserted by the NGM producer and NOT updated.
 * 3) The 'PRESENT' records of table 'native_registers_data' should be removed by the NGM consumer (C12.19 Application) after use.
 * 4) The producer should (from time to time) check if the consumer application reduces the PRESENT stack of records, and
 * 5) should stop updating it after some large value of records accumulate (e.g. 100-1000) to prevent disk space exhaustion.
 * 6) If the consumer application does not consume data then it should be restarted.
 *
 */
PRAGMA encoding = "UTF-8";
PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;

-- This table holds database version information.
CREATE TABLE db_version (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,  -- Changes each time this table is modified.
    version_major  INTEGER NOT NULL,                   -- The major version of this database schema.
    version_minor  INTEGER NOT NULL,                   -- The minor version of this database schema.
    version_min    INTEGER NOT NULL,                   -- The lowest major version number that may be used by an
                                                       -- application and be compatible with this latest schema.
    changed_date   DATETIME DEFAULT CURRENT_TIMESTAMP, -- The date when the last revision was made.
    changed_by     TEXT DEFAULT 'FDOS/ECMX UNKNOWN'    -- The name of the person that affected the change.
);

CREATE UNIQUE INDEX db_version_index
    ON db_version (version_major, version_minor);

-- Update this field to suite initial database version defaults
INSERT INTO "db_version" (id, version_major, version_minor, version_min, changed_date, changed_by) VALUES
    (NULL, 1, 0, 1, NULL, NULL) ;

-- See ANSI C12.19 UOM_ENTRY_BFLD.ID_CODE for more details.
-- This table enumerates measurement physical units. Not all ID_CODES are represented here.
-- Only ID_CODEs that will be mapped to NGM registers should be listed here
-- Those ID_CODEs that overlap with Standard ID_CODES should use snadard values.
CREATE TABLE uom_entry_id_code (
    id          INTEGER PRIMARY KEY NOT NULL,
    name        TEXT NOT NULL,
    description TEXT
);
CREATE UNIQUE INDEX uom_entry_id_code_index
    ON uom_entry_id_code (id, name);

-- See ANSI C12.19 UOM_ENTRY_BFLD.TIME_BASE  for more details.
-- This table enumerates measurement averaging method with respect to time.
CREATE TABLE uom_entry_time_base AS SELECT * FROM uom_entry_id_code;
CREATE UNIQUE INDEX uom_entry_time_base_index
    ON uom_entry_time_base (id, name);

-- See ANSI C12.19 UOM_ENTRY_BFLD.MULTIPLIER for more details.
-- This table enumerates measurement multiplier values.
CREATE TABLE uom_entry_multiplier AS SELECT * FROM uom_entry_id_code;
ALTER  TABLE uom_entry_multiplier ADD COLUMN multiplied_by DOUBLE PRECISION; -- Actual conversion factor from default.
CREATE UNIQUE INDEX uom_entry_multiplier_index
    ON uom_entry_multiplier (id, name);

-- See ANSI C12.19 UOM_ENTRY_BFLD.Qx_ACCOUNTABILITY  for more details.
-- This table enumerates commodity flow quadrants.
CREATE TABLE uom_entry_q_accountability AS SELECT * FROM uom_entry_id_code;

-- See ANSI C12.19 UOM_ENTRY_BFLD.SEGMENTATION for more details.
-- This table enumerates electrical measurement phase relations.
CREATE TABLE uom_entry_segmentation AS SELECT * FROM uom_entry_id_code;
CREATE UNIQUE INDEX uom_entry_segmentation_index
    ON uom_entry_segmentation (id, name);

-- This action inserts the maping between NGM core registers and publishd raw values.
-- Value sentered into the 'id' column are mapped into C12.19 ID_CODES.

INSERT INTO "uom_entry_id_code" (id, name, description) VALUES
    (  0, 'W',                            "Active power, Watt"),
    (  2, 'VA',                           "Apparent power (Volt-ampere), result of I RMS x V RMS."),
    (  6, 'VAR',                          "Reactive power (Var), result of SQRT(VA*VA RMS - W*W RMS)."),
    (  8, 'V',                            "Voltage RMS (Ampere)."),
    ( 12, 'I',                            "Current RMS (Volt)."),
    ( 24, 'PF',                           "Power factor (W/VA)"),
    ( 33, 'FREQ',                         "Line frequency in Hz"),
    ( 70, 'TEMPERATURE_PCB',              "PCB Temperature (Degrees C)."),
    (100, 'LATITUDE',                     "GPS latitude real time  (NFS)."),
    (101, 'LONGITUDE',                    "GPS longitude real time (NFS)."),
    (102, 'ALTITUDE',                     "GPS altitude real time. (NFS)"),
    (103, 'ACC_X',                        "Accelerometer X real time (g) (NFS)."),
    (104, 'ACC_Y',                        "Accelerometer Y real time (g) (NFS)."),
    (105, 'ACC_Z',                        "Accelerometer Z real time (g) (NFS)."), 
    (106, 'GYR_X',                        "Gyroscope X real time. (NFS)."),
    (107, 'GYR_Y',                        "Gyroscope Y real time. (NFS)."),
    (108, 'GYR_Z',                        "Gyroscope Z real time. (NFS)."),
    (217, 'TEMPERATURE_CPU',              "CPU Temperature (Degrees C).");

INSERT INTO "uom_entry_time_base" (id, name, description) VALUES
    (0, 'BULK',         "Bulk accumulated quantity of commodity (actual dials reading, e.g. Wh)."),
    (1, 'INST',         "Instantaneous (sampled). This is the fastest rate at which a measurement is acquired."),
    (2, 'PERIOD',       "Period based. Measurement based on the period of a fundamental time (e.g. once cycle RMS)."),
    (3, 'INTERVAL_SUB', "Sub-block average demand. Values from the most recent averaging demand sub-interval."),
    (4, 'INTERVAL',     "Block average Demand. Average over one or more sub-intervals."),
    (5, 'BULK_SINCE',   "Net bulk accumulated quantity of a commodity (relative dial reading since an event)."),
    (6, 'THERMAL_DEMAND', "Thermal quantity (Demand). Not Used"),
    (7, 'COUNTER',      "Event quantity (Number of occurrences of an identified item).");

-- Details the multiplier to apply to convert reading to engineering units (display)
-- id = 2 implies for W reading implies that the value is in kW
INSERT INTO "uom_entry_multiplier" (id, name, description, multiplied_by) VALUES
    (0, 'E+00',         "Multiply by 1.", 1.0),
    (1, 'E+02',         "Multiply by 100.", 100.0),
    (2, 'E+03',         "Multiply by 1,000.", 1000.0),
    (3, 'E+06',         "Multiply by 1,000,000.", 1000000.0),
    (4, 'E+09',         "Multiply by 1,000,000,000.", 1000000000.0),
    (5, 'E-02',         "Divide by 100.", 0.01),
    (6, 'E-03',         "Divide by 1,000.", 0.001),
    (7, 'E-06',         "Divide by 1,000,000.", 0.000001);

-- Details of the directio of flow filter
-- W Del = Q1 and Q4, W Rec = Q2 and Q3, VAR Del = Q1 and Q2, VAR Rec = Q3 and Q4
INSERT INTO "uom_entry_q_accountability" (id, name, description) VALUES
    (0, 'NONE',         "Quantity recording disabled"),
    (1, 'Q1',           "Quantity records in quadrant 1"),
    (2, 'Q2',           "Quantity records in quadrant 2"),
    (3, 'Q12',          "Quantity records in quadrants 1|2"),
    (4, 'Q3',           "Quantity records in quadrant 3"),
    (5, 'Q14',          "Quantity records in quadrants 1|4"),
    (6, 'Q23',          "Quantity records in quadrants 2|3"),
    (7, 'Q123',         "Quantity records in quadrants 1|2|3"),
    (8, 'Q4',           "Quantity records in quadrant 4"),
    (9, 'Q14',          "Quantity records in quadrants 1|4"),
    (10,'Q24',          "Quantity records in quadrants 2|4"),
    (11,'Q124',         "Quantity records in quadrants 1|2|4"),
    (12,'Q24',          "Quantity records in quadrants 2|4"),
    (13,'Q134',         "Quantity records in quadrants 1|3|4"),
    (14,'Q234',         "Quantity records in quadrants 2|3|4"),
    (15,'Q1234',        "Quantity records in quadrants 1|2|3|4");

-- Details the phase association of a measurement
INSERT INTO "uom_entry_segmentation" (id, name, description) VALUES
    (0, 'NONE',         "Not phase related."),     -- NGM uses this one to indicate NET/SUM all phases
    (1, 'A-B',          "Phase A to B."),
    (2, 'B-C',          "Phase B to C."),
    (3, 'C-A',          "Phase C to A."),
    (4, 'N-G',          "Neutral to ground."),
    (5, 'A-N',          "Phase A to Neutral."),    -- NGM uses this one to indicate phase A
    (6, 'B-N',          "Phase B to Neutral."),    -- NGM uses this one to indicate phase B
    (7, 'C-N',          "Phase C to Neutral.")     -- NGM uses this one to indicate phase C
    ;

-- See ANSI C12.19 UOM_ENTRY_BFLD
-- This table mirrors TABLE 12 UOM_ENTRY_TBL for convenience
-- This implementation TABLE 12 is read only and contains all possible measurements.
CREATE TABLE uom_entry_tbl (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT, -- Must match application enumerator's ordinal values.
    name                TEXT NOT NULL,
    description         TEXT,
    uom                 BIGINT,                 -- The full UINT32 value associated with the entries on the following columns.
    id_code             INTEGER NOT_NULL,       -- See uom_entry_id_code.id
    time_base           INTEGER NOT_NULL,       -- See uom_entry_time_base.id
    multiplier          INTEGER NOT_NULL,       -- See uom_entry_multiplier.id
    q_accountability    INTEGER NOT_NULL,       -- See uom_entry_q_accountability.id
    net_flow_accountability BOOLEAN NOT_NULL,
    segmentation        INTEGER DEFAULT 0,      -- See uom_entry_segmentation.id
    harmonic            BOOLEAN DEFAULT 'false',-- Set to to TRUE if an harmonic-related quantity
    nfs                 BOOLEAN DEFAULT 'false',-- Set to true to indicate NGM specialized register
    id_resource         INTEGER DEFAULT 0,      -- See uom_entry_id_resource.id (not implemented)
    
    FOREIGN KEY (id_code)          REFERENCES uom_entry_id_code(id),
    FOREIGN KEY (time_base)        REFERENCES uom_entry_time_base(id),
    FOREIGN KEY (multiplier)       REFERENCES uom_entry_multiplier(id),
    FOREIGN KEY (q_accountability) REFERENCES uom_entry_q_accountability(id),
    FOREIGN KEY (segmentation)     REFERENCES uom_entry_segmentation(id)
);
CREATE UNIQUE INDEX uom_entry_tbl_index
    ON uom_entry_tbl (id, name);

-- Reserved for future use
CREATE TABLE act_sources_lim_tbl (
    id          TEXT PRIMARY KEY NOT NULL,
    name        TEXT NOT NULL,
    description TEXT,
    value       INTEGER
);
CREATE UNIQUE INDEX act_sources_lim_tbl_index
    ON act_sources_lim_tbl (id, name);

-- Reserved for future use
CREATE TABLE demand_control_tbl AS SELECT * FROM act_sources_lim_tbl;
CREATE UNIQUE INDEX demand_control_tbl_index
    ON demand_control_tbl (id, name);

-- Reserved for future use
INSERT INTO "demand_control_tbl" (id, name, description, value) VALUES
    ('0', 'RESET_EXCLUSION',    "Number of minutes after demand reset to exclude additional reset actions.", 1),
    ('1', 'P_FAIL_RECOGNTN_TM', "Number of seconds after a power failure occurs to be recorded as power fail.", 1),
    ('2', 'P_FAIL_EXCLUSION',   "Number of minutes after a power failure to inhibit demand calculations", 1),
    ('3', 'COLD_LOAD_PICKUP',   "Number of minutes after a power failure to provide cold load pickup functions", 0),
    ('4', 'INTERVAL_VALUE',     "Each of the entries in this array is associated with an entry in the UOM_ENTRY_TBL (Table 12).", 0);

-- Fill in INTERVAL_VALUE : ARRAY[ACT_SOURCES_LIM_TBL.NBR_DEMAND_CTRL_ENTRIES] OF INT_CONTROL_RCD ;
-- This trigger will fill in one entry in demand_control_tbl for each entry in uom_entry_tbl.
-- The following generates indices id='4.x.0' and '4.x.1' where x corresponds to each id in uom_entry_tbl.
-- Entries whose UOM time_base is 3 or 4 (sub-innterval or block interval) are given non-zero sub-interval (5),
-- resulting in a default rolling interval of 15 minutes computed every 5 minutes; otherwise zero (for non
-- demand quantities).
-- Also compute the actual UINT32 UOM for reference and selection by value.
CREATE TRIGGER demand_control_tbl_trigger AFTER INSERT ON uom_entry_tbl
  BEGIN
    INSERT INTO "demand_control_tbl" (id, name, description, value) VALUES
    (('4.'||NEW.id||'.0'), 'INTERVAL_VALUE.SUB_INT', NEW.name, 
        CASE
            WHEN (NEW.time_base = 3) OR (NEW.time_base = 4) THEN 5 -- 5 minutes sub-interval.
            ELSE 0                              -- Not a demand quantity.
        END
    ),
    (('4.'||NEW.id||'.1'), 'INTERVAL_VALUE.INT_MULTIPLIER', NEW.name,
        CASE
            WHEN (NEW.time_base = 3) THEN 1	-- n minutes x 1 minutes rolling interval.
            WHEN (NEW.time_base = 4) THEN 3     -- n minutes x 3 minutes rolling interval.
            ELSE 0                              -- Not a demand quantity.
        END
    );

    -- For conveience and testing compute the actual UINT32 UOM value.
    UPDATE "uom_entry_tbl" SET uom = (
        id_code | 
        (time_base << 8) |
        (multiplier << 11) |
        (q_accountability << 14) |
        ((CASE net_flow_accountability WHEN 'true' THEN 1 ELSE 0 END) << 18) |
        ((CASE harmonic WHEN 'true' THEN 1 ELSE 0 END) << 22) |
        ((CASE nfs WHEN 'true' THEN 1 ELSE 0 END) << 31)
    ) WHERE id=NEW.id;
  END;

-- Each entry in table uom_entry_tbl has a corresponding entry in demand_control_tbl.
-- For portability and future proofing us 'name' field as a selector, as the 'id' field may change.
-- This table is to help a future application in debugging and presentation.
INSERT INTO "uom_entry_tbl" (id, name, description, id_code, time_base, multiplier, q_accountability, net_flow_accountability, segmentation, nfs) VALUES

    -- Energy all phases, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
 --   (0,     'WH_DEL',  "Active energy |delivered| from the grid to the load.",                   0, 0, 0, 9,  'false', 0, 'false'),
 --   (NULL,  'WH_REC',  "Active energy |received| from the the generator by the grid.",           0, 0, 0, 6,  'false', 0, 'false'),
    (NULL,  'WH_NET',  "Active energy (|delivered from the grid| - |received by the grid|).",    0, 0, 0, 15, 'true',  0, 'false'),
 --   (NULL,  'WH_TOT',  "Active energy (|delivered from the grid| + |received by the grid|).",    0, 0, 0, 15, 'false', 0, 'false'),
 
--    (NULL,  'VAH_DEL', "Apparent energy arithmetic while W del.",                                2, 0, 0, 9,  'false', 0, 'false'),
--    (NULL,  'VAH_REC', "Apparent energy arithmetic while W rec.",                                2, 0, 0, 6,  'false', 0, 'false'),
    (NULL,  'VAH_NET', "Apparent energy arithmetic while W net.",                                2, 0, 0, 15, 'true',  0, 'false'),
--    (NULL,  'VAH_TOT', "Apparent energy arithmetic while W tot.",                                2, 0, 0, 15, 'false', 0, 'false'),
 
--    (NULL,  'VARH_DEL',"Reactive energy vectorial while W del.",                                 6, 0, 0, 9,  'false', 0, 'false'),
--    (NULL,  'VARH_REC',"Reactive energy vectorial while W rec.",                                 6, 0, 0, 6,  'false', 0, 'false'),
    (NULL,  'VARH_NET', "Reactive energy vectorial while W net.",                                6, 0, 0, 15, 'true',  0, 'false'),
--    (NULL,  'VARH_TOT', "Reactive energy vectorial while W tot.",                                6, 0, 0, 15, 'false', 0, 'false'),

    -- Energy phase A, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    (NULL,  'WH_DEL_A',  "Active energy |delivered| from the grid to the load phase A to Neutral.",                   0, 0, 0, 9,  'false', 5, 'false'),
--    (NULL,  'WH_REC_A',  "Active energy |received| from the the generator by the grid phase A to Neutral.",           0, 0, 0, 6,  'false', 5, 'false'),
    (NULL,  'WH_NET_A',  "Active energy (|delivered from the grid| - |received by the grid|) phase A to Neutral.",    0, 0, 0, 15, 'true',  5, 'false'),
--    (NULL,  'WH_TOT_A',  "Active energy (|delivered from the grid| + |received by the grid|) phase A to Neutral.",    0, 0, 0, 15, 'false', 5, 'false'),
 
--    (NULL,  'VAH_DEL_A', "Apparent energy arithmetic while W del phase A to Neutral.",                                2, 0, 0, 9,  'false', 5, 'false'),
--    (NULL,  'VAH_REC_A', "Apparent energy arithmetic while W rec phase A to Neutral.",                                2, 0, 0, 6,  'false', 5, 'false'),
    (NULL,  'VAH_NET_A', "Apparent energy arithmetic while W net phase A to Neutral.",                                2, 0, 0, 15, 'true',  5, 'false'),
--    (NULL,  'VAH_TOT_A', "Apparent energy arithmetic while W tot phase A to Neutral.",                                2, 0, 0, 15, 'false', 5, 'false'),
 
--    (NULL,  'VARH_DEL_A',"Reactive energy vectorial while W del phase A to Neutral.",                                 6, 0, 0, 9,  'false', 5, 'false'),
--    (NULL,  'VARH_REC_A',"Reactive energy vectorial while W rec phase A to Neutral.",                                 6, 0, 0, 6,  'false', 5, 'false'),
    (NULL,  'VARH_NET_A', "Reactive energy vectorial while W net phase A to Neutral.",                                6, 0, 0, 15, 'true',  5, 'false'),
--    (NULL,  'VARH_TOT_A', "Reactive energy vectorial while W tot phase A to Neutral.",                                6, 0, 0, 15, 'false', 5, 'false'),

    -- Energy phase B, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    (NULL,  'WH_DEL_B',  "Active energy |delivered| from the grid to the load phase C to Neutral.",                   0, 0, 0, 9,  'false', 6, 'false'),
--    (NULL,  'WH_REC_B',  "Active energy |received| from the the generator by the grid phase C to Neutral.",           0, 0, 0, 6,  'false', 6, 'false'),
    (NULL,  'WH_NET_B',  "Active energy (|delivered from the grid| - |received by the grid|) phase C to Neutral.",    0, 0, 0, 15, 'true',  6, 'false'),
--    (NULL,  'WH_TOT_B',  "Active energy (|delivered from the grid| + |received by the grid|) phase C to Neutral.",    0, 0, 0, 15, 'false', 6, 'false'),
 
--    (NULL,  'VAH_DEL_B', "Apparent energy arithmetic while W del phase C to Neutral.",                                2, 0, 0, 9,  'false', 6, 'false'),
--    (NULL,  'VAH_REC_B', "Apparent energy arithmetic while W rec phase C to Neutral.",                                2, 0, 0, 6,  'false', 6, 'false'),
    (NULL,  'VAH_NET_B', "Apparent energy arithmetic while W net phase C to Neutral.",                                2, 0, 0, 15, 'true',  6, 'false'),
--    (NULL,  'VAH_TOT_B', "Apparent energy arithmetic while W tot phase C to Neutral.",                                2, 0, 0, 15, 'false', 6, 'false'),
 
 --   (NULL,  'VARH_DEL_B',"Reactive energy vectorial while W del phase C to Neutral.",                                 6, 0, 0, 9,  'false', 6, 'false'),
 --   (NULL,  'VARH_REC_B',"Reactive energy vectorial while W rec phase C to Neutral.",                                 6, 0, 0, 6,  'false', 6, 'false'),
    (NULL,  'VARH_NET_B', "Reactive energy vectorial while W net phase C to Neutral.",                                6, 0, 0, 15, 'true',  6, 'false'),
--    (NULL,  'VARH_TOT_B', "Reactive energy vectorial while W tot phase C to Neutral.",                                6, 0, 0, 15, 'false', 6, 'false'),

    -- Energy phase C, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    (NULL,  'WH_DEL_C',  "Active energy |delivered| from the grid to the load phase C to Neutral.",                   0, 0, 0, 9,  'false', 7, 'false'),
--    (NULL,  'WH_REC_C',  "Active energy |received| from the the generator by the grid phase C to Neutral.",           0, 0, 0, 6,  'false', 7, 'false'),
    (NULL,  'WH_NET_C',  "Active energy (|delivered from the grid| - |received by the grid|) phase C to Neutral.",    0, 0, 0, 15, 'true',  7, 'false'),
 --   (NULL,  'WH_TOT_C',  "Active energy (|delivered from the grid| + |received by the grid|) phase C to Neutral.",    0, 0, 0, 15, 'false', 7, 'false'),
 
 --   (NULL,  'VAH_DEL_C', "Apparent energy arithmetic while W del phase C to Neutral.",                                2, 0, 0, 9,  'false', 7, 'false'),
 --   (NULL,  'VAH_REC_C', "Apparent energy arithmetic while W rec phase C to Neutral.",                                2, 0, 0, 6,  'false', 7, 'false'),
    (NULL,  'VAH_NET_C', "Apparent energy arithmetic while W net phase C to Neutral.",                                2, 0, 0, 15, 'true',  7, 'false'),
--    (NULL,  'VAH_TOT_C', "Apparent energy arithmetic while W tot phase C to Neutral.",                                2, 0, 0, 15, 'false', 7, 'false'),
 
--    (NULL,  'VARH_DEL_C',"Reactive energy vectorial while W del phase C to Neutral.",                                 6, 0, 0, 9,  'false', 7, 'false'),
--    (NULL,  'VARH_REC_C',"Reactive energy vectorial while W rec phase C to Neutral.",                                 6, 0, 0, 6,  'false', 7, 'false'),
    (NULL,  'VARH_NET_C', "Reactive energy vectorial while W net phase C to Neutral.",                                6, 0, 0, 15, 'true',  7, 'false'),
--    (NULL,  'VARH_TOT_C', "Reactive energy vectorial while W tot phase C to Neutral.",                                6, 0, 0, 15, 'false', 7, 'false'),

    -- Real time all phases power and readings (in this implementation we mean average 1s).
--    (NULL,  'W_DEL',  "Active power |del| real time.",                                                                0, 2, 0, 9,  'false', 0, 'false'),
--    (NULL,  'W_REC',  "Active power |rec| real time.",                                                                0, 2, 0, 6,  'false', 0, 'false'),
    (NULL,  'W_NET',  "Active power (|del| - |rec|) real time.",                                                      0, 2, 0, 15, 'true',  0, 'false'),
--    (NULL,  'W_TOT',  "Active power (|del| + |rec|) real time.",                                                      0, 2, 0, 15, 'false', 0, 'false'),

--    (NULL,  'VA_DEL',  "Apparent energy arithmetic while W del real time.",                                           2, 2, 0, 9,  'false', 0, 'false'),
--    (NULL,  'VA_REC',  "Apparent energy arithmetic while W rec real time.",                                           2, 2, 0, 6,  'false', 0, 'false'),
    (NULL,  'VA_NET',  "Apparent energy arithmetic while W net real time.",                                           2, 2, 0, 15, 'true',  0, 'false'),
 --   (NULL,  'VA_TOT',  "Apparent energy arithmetic while W tot real time.",                                           2, 2, 0, 15, 'false', 0, 'false'),

 --   (NULL,  'VAR_DEL', "Reactive energy vectorial while W del real time.",                                            4, 2, 0, 9,  'false', 0, 'false'),
 --   (NULL,  'VAR_REC', "Reactive energy vectorial while W rec real time.",                                            4, 2, 0, 6,  'false', 0, 'false'),
    (NULL,  'VAR_NET', "Reactive energy vectorial while W net real time.",                                            4, 2, 0, 15, 'true',  0, 'false'),
--    (NULL,  'VAR_TOT', "Reactive energy vectorial while W tot real time.",                                            4, 2, 0, 15, 'false', 0, 'false'),

    -- Real time phase A power and readings (in this implementation we mean average 1s).
--    (NULL,  'W_DEL_A',  "Active power |del| real time phase A to Neutral.",                                           0, 2, 0, 9,  'false', 5, 'false'),
--    (NULL,  'W_REC_A',  "Active power |rec| real time phase A to Neutral.",                                           0, 2, 0, 6,  'false', 5, 'false'),
    (NULL,  'W_NET_A',  "Active power (|del| - |rec|) real time phase A to Neutral.",                                 0, 2, 0, 15, 'true',  5, 'false'),
--    (NULL,  'W_TOT_A',  "Active power (|del| + |rec|) real time phase A to Neutral.",                                 0, 2, 0, 15, 'false', 5, 'false'),

 --   (NULL,  'VA_DEL_A',  "Apparent energy arithmetic while W del real time phase A to Neutral.",                      2, 2, 0, 9,  'false', 5, 'false'),
 --   (NULL,  'VA_REC_A',  "Apparent energy arithmetic while W rec real time phase A to Neutral.",                      2, 2, 0, 6,  'false', 5, 'false'),
    (NULL,  'VA_NET_A',  "Apparent energy arithmetic while W net real time phase A to Neutral.",                      2, 2, 0, 15, 'true',  5, 'false'),
--    (NULL,  'VA_TOT_A',  "Apparent energy arithmetic while W tot real time phase A to Neutral.",                      2, 2, 0, 15, 'false', 5, 'false'),

 --   (NULL,  'VAR_DEL_A', "Reactive energy vectorial while W del real time phase A to Neutral.",                       4, 2, 0, 9,  'false', 5, 'false'),
--    (NULL,  'VAR_REC_A', "Reactive energy vectorial while W rec real time phase A to Neutral.",                       4, 2, 0, 6,  'false', 5, 'false'),
    (NULL,  'VAR_NET_A', "Reactive energy vectorial while W net real time phase A to Neutral.",                       4, 2, 0, 15, 'true',  5, 'false'),
--    (NULL,  'VAR_TOT_A', "Reactive energy vectorial while W tot real time phase A to Neutral.",                       4, 2, 0, 15, 'false', 5, 'false'),
 
    (NULL,  'FREQ_A',   "Line frequency in Hz real time phase A to Neutral.",                                        33, 2, 0, 15, 'false', 5, 'false'), 
    (NULL,  'V_A',      "RMS voltage real time phase A to Neutral.",                                                  8, 2, 0, 15, 'false', 5, 'false'),

--    (NULL,  'I_DEL_A',  "RMS current while W del real time phase A to Neutral.",                                     12, 2, 0, 9,  'false', 5, 'false'),
--    (NULL,  'I_REC_A',  "RMS current while W rec real time phase A to Neutral.",                                     12, 2, 0, 6,  'false', 5, 'false'),
    (NULL,  'I_NET_A',  "RMS current while W net real time phase A to Neutral.",                                     12, 2, 0, 15, 'true',  5, 'false'),
--    (NULL,  'I_TOT_A',  "RMS current while W tot real time phase A to Neutral.",                                     12, 2, 0, 15, 'false', 5, 'false'),

--    (NULL,  'PF_DEL_A',  "Power factor while W del real time phase A to Neutral.",                                   24, 2, 0, 9,  'false', 5, 'false'),
 --   (NULL,  'PF_REC_A',  "Power factor while W rec real time phase A to Neutral.",                                   24, 2, 0, 6,  'false', 5, 'false'),
    (NULL,  'PF_NET_A',  "Power factor while W net real time phase A to Neutral.",                                   24, 2, 0, 15, 'false', 5, 'false'),
--    (NULL,  'PF_TOT_A',  "Power factor while W tot real time phase A to Neutral.",                                   24, 2, 0, 15, 'false', 5, 'false'),
 
   -- Real time phase B power and readings (in this implementation we mean average 1s).
--    (NULL,  'W_DEL_B',  "Active power |del| real time phase B to Neutral.",                                           0, 2, 0, 9,  'false', 6, 'false'),
--    (NULL,  'W_REC_B',  "Active power |rec| real time phase B to Neutral.",                                           0, 2, 0, 6,  'false', 6, 'false'),
    (NULL,  'W_NET_B',  "Active power (|del| - |rec|) real time phase B to Neutral.",                                 0, 2, 0, 15, 'true',  6, 'false'),
--    (NULL,  'W_TOT_B',  "Active power (|del| + |rec|) real time phase B to Neutral.",                                 0, 2, 0, 15, 'false', 6, 'false'),

--    (NULL,  'VA_DEL_B',  "Apparent energy arithmetic while W del real time phase B to Neutral.",                      2, 2, 0, 9,  'false', 6, 'false'),
--    (NULL,  'VA_REC_B',  "Apparent energy arithmetic while W rec real time phase B to Neutral.",                      2, 2, 0, 6,  'false', 6, 'false'),
    (NULL,  'VA_NET_B',  "Apparent energy arithmetic while W net real time phase B to Neutral.",                      2, 2, 0, 15, 'true',  6, 'false'),
 --   (NULL,  'VA_TOT_B',  "Apparent energy arithmetic while W tot real time phase B to Neutral.",                      2, 2, 0, 15, 'false', 6, 'false'),

 --   (NULL,  'VAR_DEL_B', "Reactive energy vectorial while W del real time phase B to Neutral.",                       4, 2, 0, 9,  'false', 6, 'false'),
--    (NULL,  'VAR_REC_B', "Reactive energy vectorial while W rec real time phase B to Neutral.",                       4, 2, 0, 6,  'false', 6, 'false'),
    (NULL,  'VAR_NET_B', "Reactive energy vectorial while W net real time phase B to Neutral.",                       4, 2, 0, 15, 'true',  6, 'false'),
--    (NULL,  'VAR_TOT_B', "Reactive energy vectorial while W tot real time phase B to Neutral.",                       4, 2, 0, 15, 'false', 6, 'false'),
 
    (NULL,  'FREQ_B',   "Line frequency in Hz real time phase B to Neutral.",                                        33, 2, 0, 15, 'false', 6, 'false'), 
    (NULL,  'V_B',      "RMS voltage real time phase B to Neutral.",                                                  8, 2, 0, 15, 'false', 6, 'false'),

--    (NULL,  'I_DEL_B',  "RMS current while W del real time phase B to Neutral.",                                     12, 2, 0, 9,  'false', 6, 'false'),
--    (NULL,  'I_REC_B',  "RMS current while W rec real time phase B to Neutral.",                                     12, 2, 0, 6,  'false', 6, 'false'),
    (NULL,  'I_NET_B',  "RMS current while W net real time phase B to Neutral.",                                     12, 2, 0, 15, 'true',  6, 'false'),
--    (NULL,  'I_TOT_B',  "RMS current while W tot real time phase B to Neutral.",                                     12, 2, 0, 15, 'false', 6, 'false'),

--    (NULL,  'PF_DEL_B',  "Power factor while W del real time phase B to Neutral.",                                   24, 2, 0, 9,  'false', 6, 'false'),
--    (NULL,  'PF_REC_B',  "Power factor while W rec real time phase B to Neutral.",                                   24, 2, 0, 6,  'false', 6, 'false'),
    (NULL,  'PF_NET_B',  "Power factor while W net real time phase B to Neutral.",                                   24, 2, 0, 15, 'false', 6, 'false'),
--    (NULL,  'PF_TOT_B',  "Power factor while W tot real time phase B to Neutral.",                                   24, 2, 0, 15, 'false', 6, 'false'),

  -- Real time phase C power and readings (in this implementation we mean average 1s).
--    (NULL,  'W_DEL_C',  "Active power |del| real time phase B to Neutral.",                                           0, 2, 0, 9,  'false', 7, 'false'),
--    (NULL,  'W_REC_C',  "Active power |rec| real time phase B to Neutral.",                                           0, 2, 0, 6,  'false', 7, 'false'),
    (NULL,  'W_NET_C',  "Active power (|del| - |rec|) real time phase B to Neutral.",                                 0, 2, 0, 15, 'true',  7, 'false'),
--    (NULL,  'W_TOT_C',  "Active power (|del| + |rec|) real time phase B to Neutral.",                                 0, 2, 0, 15, 'false', 7, 'false'),

--    (NULL,  'VA_DEL_C',  "Apparent energy arithmetic while W del real time phase B to Neutral.",                      2, 2, 0, 9,  'false', 7, 'false'),
--    (NULL,  'VA_REC_C',  "Apparent energy arithmetic while W rec real time phase B to Neutral.",                      2, 2, 0, 6,  'false', 7, 'false'),
    (NULL,  'VA_NET_C',  "Apparent energy arithmetic while W net real time phase B to Neutral.",                      2, 2, 0, 15, 'true',  7, 'false'),
--    (NULL,  'VA_TOT_C',  "Apparent energy arithmetic while W tot real time phase B to Neutral.",                      2, 2, 0, 15, 'false', 7, 'false'),

--    (NULL,  'VAR_DEL_C', "Reactive energy vectorial while W del real time phase B to Neutral.",                       4, 2, 0, 9,  'false', 7, 'false'),
--    (NULL,  'VAR_REC_C', "Reactive energy vectorial while W rec real time phase B to Neutral.",                       4, 2, 0, 6,  'false', 7, 'false'),
    (NULL,  'VAR_NET_C', "Reactive energy vectorial while W net real time phase B to Neutral.",                       4, 2, 0, 15, 'true',  7, 'false'),
--    (NULL,  'VAR_TOT_C', "Reactive energy vectorial while W tot real time phase B to Neutral.",                       4, 2, 0, 15, 'false', 7, 'false'),
 
    (NULL,  'FREQ_C',   "Line frequency in Hz real time phase B to Neutral.",                                        33, 2, 0, 15, 'false', 7, 'false'), 
    (NULL,  'V_C',      "RMS voltage real time phase B to Neutral.",                                                  8, 2, 0, 15, 'false', 7, 'false'),

--    (NULL,  'I_DEL_C',  "RMS current while W del real time phase B to Neutral.",                                     12, 2, 0, 9,  'false', 7, 'false'),
--    (NULL,  'I_REC_C',  "RMS current while W rec real time phase B to Neutral.",                                     12, 2, 0, 6,  'false', 7, 'false'),
    (NULL,  'I_NET_C',  "RMS current while W net real time phase B to Neutral.",                                     12, 2, 0, 15, 'true',  7, 'false'),
--    (NULL,  'I_TOT_C',  "RMS current while W tot real time phase B to Neutral.",                                     12, 2, 0, 15, 'false', 7, 'false'),

--    (NULL,  'PF_DEL_C',  "Power factor while W del real time phase B to Neutral.",                                   24, 2, 0, 9,  'false', 7, 'false'),
--    (NULL,  'PF_REC_C',  "Power factor while W rec real time phase B to Neutral.",                                   24, 2, 0, 6,  'false', 7, 'false'),
    (NULL,  'PF_NET_C',  "Power factor while W net real time phase B to Neutral.",                                   24, 2, 0, 15, 'false', 7, 'false'),
--    (NULL,  'PF_TOT_C',  "Power factor while W tot real time phase B to Neutral.",                                   24, 2, 0, 15, 'false', 7, 'false'),


    -- Miscalaneous registers... e.g. GPS and Accelerations

    (NULL,  'LATITUDE',   "GPS latitude real time.",                                                                  100, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'LONGITUDE', "GPS longitude real time.",                                                                 101, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'ALTITUDE',  "GPS altitude real time.",                                                                  102, 2, 0, 15, 'false', 0, 'true'),

    (NULL,  'ACC_X',     "Accelerometer X real time (g).",                                                           103, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'ACC_Y',     "Accelerometer Y real time (g).",                                                           104, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'ACC_Z',     "Accelerometer Z real time (g).",                                                           105, 2, 0, 15, 'false', 0, 'true'),
 
    (NULL,  'GYR_X',     "Gyroscope X real time.",                                                                   106, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'GYR_Y',     "Gyroscope Y real time.",                                                                   107, 2, 0, 15, 'false', 0, 'true'),
    (NULL,  'GYR_Z',     "Gyroscope Z real time.",                                                                   108, 2, 0, 15, 'false', 0, 'true'),

    (NULL,  'TEMP_PCB', "CPU temperature (Deg. C) real time.",                                                        70, 2, 0, 15, 'false', 0, 'false'),
    (NULL,  'TEMP_CPU', "PCB temperature (Deg. C) real time.",                                                       217, 2, 0, 15, 'false', 0, 'false');


    -- Demand interval average power and readings (across all demand sub-intervals) not implemented by NGM.
    -- Demand sub-interval average power and readings (across one demand sub-interval) not implemented by NGM.

    -- Events and counters
    -- TODO: Remove unused counters and add extra counters and status
/*
    (NULL,  'COUNT_OUTAGE',         "Number of power outages.",                                 50, 7, 0, 15, 'false', 0, 'false'),
    (NULL,  'COUNT_DEMAND_RESET',   "Number of demand resets.",                                 51, 7, 0, 15, 'false', 0, 'false'),
    (NULL,  'COUNT_PROGRAMMED',     "Number of time programmed.",                               52, 7, 0, 15, 'false', 0, 'false'),
    (NULL,  'TIME_ON_BATTERY',      "Time on battery (minutes).",                               53, 0, 0, 15, 'false', 0, 'false'),
    (NULL,  'COUNT_WD_TIMEOUT',     "Number of watch-dog resets.",                              61, 7, 0, 15, 'false', 0, 'false'),
    (NULL,  'COUNT_ALARMS',         "Number of diagnostic alarms.",                             62, 7, 0, 15, 'false', 0, 'false'),
    (NULL,  'DURATION_ZERO_FLOW',   "Zero flow duration (minutes).",                            39, 0, 0, 15, 'false', 0, 'false'),
    (NULL,  'TIME_OUTAGE',          "Time of last power outage (system mains).",                42, 1, 0, 15, 'false', 0, 'false'),
    (NULL,  'TIME_OUTAGE_RESTORED', "Time of power outage restoration  (system mains).",        42, 1, 0, 15, 'false', 0, 'false');
*/

-- We insert into act_sources_lim_tbl after insert into uom_entry_tbl due to dependency on uom_entry_tbl
INSERT INTO "act_sources_lim_tbl" (id, name, description, value) VALUES 
    -- Bit values for SOURCE_FLAGS:
    --   1: PF_EXCLUDE_FLAG
    --   2: RESET_EXCLUDE_FLAG
    --   4: BLOCK_DEMAND_FLAG
    --   8: SLIDING_DEMAND_FLAG
    --  16: THERMAL_DEMAND_FLAG
    --  32: SET1_PRESENT_FLAG
    --  64: SET2_PRESENT_FLAG
    -- 128: CONVERSION_ALG_FLAG (per Annex K of ANSI C12.19 V2.0 or later).
    ('0', 'SOURCE_FLAGS',            "See SOURCE_FLAGS_BFLD.",                          1+2+8+64+128),
    ('1', 'NBR_UOM_ENTRIES',         "Number of entries in UOM_ENTRY_TBL.",             (SELECT COUNT(*) FROM uom_entry_tbl)),
    ('2', 'NBR_DEMAND_CTRL_ENTRIES', "Number of entries in DEMAND_CONTROL_TBL.",        (SELECT COUNT(*) FROM uom_entry_tbl)),
    ('3', 'DATA_CTRL_LENGTH',        "Size of SOURCE_ID entry in DATA_CONTROL_TBL.",    0),
    ('4', 'NBR_DATA_CTRL_ENTRIES',   "Number of entries in DATA_CONTROL_TBL.",          0),
    ('5', 'NBR_CONSTANTS_ENTRIES',   "Number of entries in CONSTANTS_TBL.",             0),
    ('6', 'CONSTANTS_SELECTOR',      "ELECTRIC_CONSTANTS_RCD data structure selected.", 2),
    -- Note: ANSI C12.19 Annex states that  when Table 16 (SOURCES_TBL) is not retrievable
    --       from the End Device (meter) or from the an EDL file, then there shall be an assumed
    --       correspondence of 1:1 between each entry index in Table 16 (SOURCES_TBL) and index
    --       into the corresponding entry in Table 12 through Table 15 (UOM_ENTRY_TBL, 
    --       DEMAND_CONTROL_TBL, DATA_CONTROL_TBL and through CONSTANTS_TBL).
    --       However, Table 16 (SOURCES_TBL) default values (assuming that not present)
    --       may be take
    ('7', 'NBR_SOURCES',             "Number of entries in SOURCES_TBL.",               (SELECT COUNT(*) FROM uom_entry_tbl)) ;

-- Given that demand_control_tbl is READ_ONLY we can drop the trigger
-- If in the future this changes then need to add update and delete triggers.
DROP TRIGGER demand_control_tbl_trigger;

-- Reserved for future use
CREATE TABLE demand_qualifier (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    name                TEXT NOT NULL,
    description         TEXT
);
CREATE UNIQUE INDEX demand_qualifier_index 
    ON demand_qualifier (id, name);

-- Persistent register data qualifiers. NGM only supports 'PRESENT' and 'TOTAL'
INSERT INTO "demand_qualifier" VALUES
    (0,'PRESENT',   'Repersents a quantity that is ready for presentation e.g. 1 minute average value or a counter.'),
    (1,'INTERVAL',  'Repersents an average value over a demand interval (e.g. 15 minutes).'),
    (2,'MINIMUM',   'Repersents an smallest average value based on a demand interval (INTERVAL) since last demand reset.'),
    (3,'MAXIMUM',   'Repersents an largest average value based on a demand interval (INTERVAL) since last demand reset.'),
    (4,'TOTAL',     'Represents a totalizer set (dial readings) baseline');

-- Reserved for future use.
CREATE TABLE demand_reset (
    id          INTEGER PRIMARY KEY AUTOINCREMENT, -- maximum of this is the number of demand resets performed.
    timestamp   BIGINT NOT NULL   -- The time when a demand reset took place recorded in milliseconds since 1970.
);
INSERT INTO "demand_reset" VALUES (0,0) ;

CREATE TABLE native_registers_data (
-- Native (NGM non C12) persistent data registers restored on initialization.
-- These registers must be placed in a database file system (e.g. flash) that is resilient to repeated update.

    id                  INTEGER NOT NULL,   -- This is the number that groups related peak demands (qualifier = 2|3)
                                            -- The present peak demand qualifier must equal demand_reset.id (number of demand resets).
    qualifier           INTEGER NOT NULL,   -- This is the demand average qualifier see demand_qualifier.
    timestamp           BIGINT  NOT NULL,   -- The Network time when this register group was recorded in milliseconds since 1970 UTC.
    interval            BIGINT,             -- Averaging interval duration in milliseconds.
    associate           TEXT,               -- If MIN or MAX then this is the name of the column that is the MIN or MAX values.
                                            -- Otherwise the UOM name that was the trigger for the dataset (e.g. power restore).

    -- Values below one per column are core UOM.
    -- Totals are updated each time an average is updated, so that a single row for a given id contains related data.
    -- Together with interval qualifier a proper UOM can be reconstructed.
    -- Energy all phases, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
    -- Columns are added automatically by a TRIGGER on insert within uom_entry_tbl.

    -- Energy all phases, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    'WH_DEL'        DOUBLE,
--    'WH_REC'        DOUBLE,
    'WH_NET'        DOUBLE,
--    'WH_TOT'        DOUBLE,
--    'VAH_DEL'        DOUBLE,
--    'VAH_REC'        DOUBLE,
    'VAH_NET'        DOUBLE,
--    'VAH_TOT'        DOUBLE,
--    'VARH_DEL'        DOUBLE,
--    'VARH_REC'        DOUBLE,
    'VARH_NET'        DOUBLE,
--    'VARH_TOT'        DOUBLE,

    -- Energy phase A, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    'WH_DEL_A'        DOUBLE,
--    'WH_REC_A'        DOUBLE,
    'WH_NET_A'        DOUBLE,
--    'WH_TOT_A'        DOUBLE,
--    'VAH_DEL_A'        DOUBLE,
--    'VAH_REC_A'        DOUBLE,
    'VAH_NET_A'        DOUBLE,
--    'VAH_TOT_A'        DOUBLE,
--    'VARH_DEL_A'        DOUBLE,
--    'VARH_REC_A'        DOUBLE,
    'VARH_NET_A'        DOUBLE,
--    'VARH_TOT_A'        DOUBLE,

    -- Energy phase B, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    'WH_DEL_B'        DOUBLE,
--    'WH_REC_B'        DOUBLE,
    'WH_NET_B'        DOUBLE,
--    'WH_TOT_B'        DOUBLE,
 
--    'VAH_DEL_B'        DOUBLE,
--    'VAH_REC_B'        DOUBLE,
    'VAH_NET_B'        DOUBLE,
--    'VAH_TOT_B'        DOUBLE,
--    'VARH_DEL_B'        DOUBLE,
--    'VARH_REC_B'        DOUBLE,
    'VARH_NET_B'        DOUBLE,
--    'VARH_TOT_B'        DOUBLE,

    -- Energy phase C, see ANSI C12.19 for clarity on what "W del|rec|net|tot" mean.
--    'WH_DEL_C'        DOUBLE,
--    'WH_REC_C'        DOUBLE,
    'WH_NET_C'        DOUBLE,
--    'WH_TOT_C'        DOUBLE,

--    'VAH_DEL_C'        DOUBLE,
--    'VAH_REC_C'        DOUBLE,
    'VAH_NET_C'        DOUBLE,
--    'VAH_TOT_C'        DOUBLE,
 
--    'VARH_DEL_C'        DOUBLE,
--    'VARH_REC_C'        DOUBLE,
    'VARH_NET_C'        DOUBLE,
--    'VARH_TOT_C'        DOUBLE,

    -- Real time all phases power and readings
--    'W_DEL'        DOUBLE,
--    'W_REC'        DOUBLE,
    'W_NET'        DOUBLE,
--    'W_TOT'        DOUBLE,
--    'VA_DEL'        DOUBLE,
--    'VA_REC'        DOUBLE,
    'VA_NET'        DOUBLE,
--    'VA_TOT'        DOUBLE,

--    'VAR_DEL'        DOUBLE,
--    'VAR_REC'        DOUBLE,
    'VAR_NET'        DOUBLE,
--    'VAR_TOT'        DOUBLE,

    -- Real time phase A power and readings
--    'W_DEL_A'        DOUBLE,
--    'W_REC_A'        DOUBLE,
    'W_NET_A'        DOUBLE,
--    'W_TOT_A'        DOUBLE,
--    'VA_DEL_A'        DOUBLE,
--    'VA_REC_A'        DOUBLE,
    'VA_NET_A'        DOUBLE,
--    'VA_TOT_A'        DOUBLE,
--    'VAR_DEL_A'        DOUBLE,
--    'VAR_REC_A'        DOUBLE,
    'VAR_NET_A'        DOUBLE,
--    'VAR_TOT_A'        DOUBLE,

    'FREQ_A'        DOUBLE,
    'V_A'           DOUBLE,

--    'I_DEL_A'        DOUBLE,
--    'I_REC_A'        DOUBLE,
    'I_NET_A'        DOUBLE,
--    'I_TOT_A'        DOUBLE,

--    'PF_DEL_A'        DOUBLE,
--    'PF_REC_A'        DOUBLE,
    'PF_NET_A'        DOUBLE,
--    'PF_TOT_A'        DOUBLE,
 
   -- Real time phase B power and readings
--    'W_DEL_B'        DOUBLE,
--    'W_REC_B'        DOUBLE,
    'W_NET_B'        DOUBLE,
--    'W_TOT_B'        DOUBLE,

--    'VA_DEL_B'        DOUBLE,
--    'VA_REC_B'        DOUBLE,
    'VA_NET_B'        DOUBLE,
 --   'VA_TOT_B'        DOUBLE,

--    'VAR_DEL_B'        DOUBLE,
--    'VAR_REC_B'        DOUBLE,
    'VAR_NET_B'        DOUBLE,
--    'VAR_TOT_B'        DOUBLE,
 
    'FREQ_B'        DOUBLE,
    'V_B'           DOUBLE,

--    'I_DEL_B'        DOUBLE,
--    'I_REC_B'        DOUBLE,
    'I_NET_B'        DOUBLE,
--    'I_TOT_B'        DOUBLE,

--    'PF_DEL_B'        DOUBLE,
--    'PF_REC_B'        DOUBLE,
    'PF_NET_B'        DOUBLE,
--    'PF_TOT_B'        DOUBLE,

  -- Real time phase C power and readings
--    'W_DEL_C'        DOUBLE,
--    'W_REC_C'        DOUBLE,
    'W_NET_C'        DOUBLE,
--    'W_TOT_C'        DOUBLE,

--    'VA_DEL_C'        DOUBLE,
--    'VA_REC_C'        DOUBLE,
    'VA_NET_C'        DOUBLE,
--    'VA_TOT_C'        DOUBLE,

--    'VAR_DEL_C'        DOUBLE,
--    'VAR_REC_C'        DOUBLE,
    'VAR_NET_C'        DOUBLE,
--    'VAR_TOT_C'        DOUBLE,
 
    'FREQ_C'        DOUBLE,
    'V_C'           DOUBLE,

--    'I_DEL_C'        DOUBLE,
--    'I_REC_C'        DOUBLE,
    'I_NET_C'        DOUBLE,
--    'I_TOT_C'        DOUBLE,

--    'PF_DEL_C'        DOUBLE,
--    'PF_REC_C'        DOUBLE,
    'PF_NET_C'        DOUBLE,
--    'PF_TOT_C'        DOUBLE,

    -- Miscalaneous registers... e.g. GPS and Accelerations

    'LATITUDE'        DOUBLE,
    'LONGITUDE'       DOUBLE,
    'ALTITUDE'        DOUBLE,

    'ACC_X'           DOUBLE,
    'ACC_Y'           DOUBLE,
    'ACC_Z'           DOUBLE,
 
    'GYR_X'           DOUBLE,
    'GYR_Y'           DOUBLE,
    'GYR_Z'           DOUBLE,

    'TEMP_PCB'        DOUBLE,
    'TEMP_CPU'        DOUBLE,

    -- Demand interval average power and readings
/* 
    'COUNT_OUTAGE'            DOUBLE,
    'COUNT_DEMAND_RESET'      DOUBLE,
    'COUNT_PROGRAMMED'        DOUBLE,
    'TIME_ON_BATTERY'         DOUBLE,
    'COUNT_WD_TIMEOUT'        DOUBLE,
    'COUNT_ALARMS'            DOUBLE,
    'DURATION_ZERO_FLOW'      DOUBLE,
    'TIME_OUTAGE'             DOUBLE,
    'TIME_OUTAGE_RESTORED'    DOUBLE,
*/
     FOREIGN KEY (qualifier)     REFERENCES demand_qualifier(id)
);

CREATE UNIQUE INDEX native_registers_data_index
    ON native_registers_data (id, qualifier, associate);

-- Example data
INSERT INTO "native_registers_data" (id,qualifier,timestamp,interval,associate,
            'WH_NET','TEMP_PCB', 'TEMP_CPU') VALUES
 (0,4,1494370770119,1000,'BASELINE', 1605422.733, 43.201,72.532) ,
 (0,4,1494370770119,1000,'PRESENT',  1605422.123, 42.750,73.292) 
;

COMMIT;
