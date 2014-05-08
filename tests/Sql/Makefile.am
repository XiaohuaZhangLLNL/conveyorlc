
if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = TestSql TestSqlDataBase TestSqlField TestSqlFieldSet TestSqlRecord TestSqlRecordSet TestSqlTable TestSqlValue
else
endif

if USE_MPI
    TestSql_SOURCES = TestSql.cpp
    TestSql_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlDataBase_SOURCES = TestSqlDataBase.cpp
    TestSqlDataBase_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlField_SOURCES = TestSqlField.cpp
    TestSqlField_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlFieldSet_SOURCES = TestSqlFieldSet.cpp
    TestSqlFieldSet_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlRecord_SOURCES = TestSqlRecord.cpp
    TestSqlRecord_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlRecordSet_SOURCES = TestSqlRecordSet.cpp
    TestSqlRecordSet_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlTable_SOURCES = TestSqlTable.cpp
    TestSqlTable_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

    TestSqlValue_SOURCES = TestSqlValue.cpp
    TestSqlValue_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/usr/lib/ -lsqlite3

else
endif
