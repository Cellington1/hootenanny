# -------------------------------------------------
# The Hoot R&D project is a place to store experimental code that isn't quite ready for prime time.
# At this point anthing below TRL5 [1] should be put in this project. This project is disabled by
# default which should help avoid introducing bugs into the core while experimenting.
#
# 1. TRL5 - Module and/or subsystem validation in a relevant environment
# -------------------------------------------------
QT += script \
    sql \
    testlib \
    xml \

TARGET = HootRnd
TEMPLATE = lib

DEPENDPATH += \
    ../tbs/src/main/cpp/ \
    ../tgs/src/main/cpp/ \
    ../hoot-core/src/main/cpp/ \
    ../hoot-core-test/src/test/cpp/ \
    src/main/cpp/ \

INCLUDEPATH += \
  $${DEPENDPATH} \
  /usr/include/nodejs \
  ../local/include/ \

CONFIG += rtti \
    qtestlib \
    debug

include(../Configure.pri)

QMAKE_CXXFLAGS = -I/usr/include/nodejs $$QMAKE_CXXFLAGS

LIBS += -L../lib/ -lTgs -ltbs -lHootCore

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
release:OBJECTS_DIR = tmp/release
else:OBJECTS_DIR = tmp/debug
DESTDIR = ../lib/

cppunit {
SOURCES += \

HEADERS += \

}

SOURCES += \
    src/test/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatchCreatorTest.cpp \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatchCreator.cpp \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatch.cpp \
    src/test/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatchTest.cpp \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMerger.cpp \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMergerCreator.cpp \
    src/test/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMergerTest.cpp \
    src/test/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMergerCreatorTest.cpp \
    src/main/cpp/hoot/rnd/ops/KeepPoiAndBuildingRefOp.cpp \
    src/test/cpp/hoot/rnd/visitors/MatchCandidateCountVisitorRndTest.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMatchCreator.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMatch.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMerger.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMergerCreator.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkEdge.cpp \
    src/main/cpp/hoot/rnd/conflate/network/OsmNetworkExtractor.cpp \
    src/main/cpp/hoot/rnd/conflate/network/OsmNetwork.cpp \
    src/main/cpp/hoot/rnd/conflate/network/IterativeNetworkMatcher.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkEdgeMatch.cpp \
    src/main/cpp/hoot/rnd/conflate/network/NetworkVertex.cpp

HEADERS += \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatchCreator.h \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMatch.h \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMerger.h \
    src/main/cpp/hoot/rnd/conflate/poi-polygon/PoiPolygonMergerCreator.h \
    src/main/cpp/hoot/rnd/ops/KeepPoiAndBuildingRefOp.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMatchCreator.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMatch.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMerger.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkMergerCreator.h \
    src/main/cpp/hoot/rnd/conflate/network/OsmNetworkExtractor.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkEdge.h \
    src/main/cpp/hoot/rnd/conflate/network/OsmNetwork.h \
    src/main/cpp/hoot/rnd/conflate/network/IterativeNetworkMatcher.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkEdgeMatch.h \
    src/main/cpp/hoot/rnd/conflate/network/NetworkVertex.h

FORMS +=

OTHER_FILES +=
