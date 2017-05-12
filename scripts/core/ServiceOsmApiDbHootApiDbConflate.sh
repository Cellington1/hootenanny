#!/bin/bash
set -e

# This is a base test script for conflating datasets where one dataset comes from an osm api database and the other from 
# a hoot api database.  It simulates end to end at the command line level what one possible conflation workflow with MapEdit data 
# looks like.  See ServiceOsmApiDbHootApiDbAllDataTypesConflateTest.sh for an example of how to call this script.
# 
# This script:
#   - writes two datasets, one to an OSM API database and one to a Hoot API database; assumes the two have overlapping aoi's 
#   - reads in the same subset AOI from each dataset
#   - conflates the data together and writes the result to a Hoot API database
#   - writes out a sql changeset file that is the difference between the original OSM API database reference dataset and the conflated output in the Hoot API database
#   - executes the changeset file SQL against the OSM API database
#   - reads out the entire contents of the OSM API database and verifies it

REF_DATASET=$1
SEC_DATASET=$2
AOI=$3
#AOI=-180,-90,180,90 # for debugging
TEST_NAME=$4

echo "reference dataset: " $REF_DATASET
echo "secondary dataset: " $SEC_DATASET
echo "AOI: " $AOI
echo "TEST_NAME: " $TEST_NAME

RUN_DEBUG_STEPS=false

# set to false for testing only
LOAD_REF_DATA=true
LOAD_SEC_DATA=true
CONFLATE_DATA=true

source conf/database/DatabaseConfig.sh
export OSM_API_DB_URL="osmapidb://$DB_USER:$DB_PASSWORD@$DB_HOST:$DB_PORT/$DB_NAME_OSMAPI"
export OSM_API_DB_AUTH="-h $DB_HOST -p $DB_PORT -U $DB_USER"
export PGPASSWORD=$DB_PASSWORD_OSMAPI
HOOT_DB_URL="hootapidb://$DB_USER:$DB_PASSWORD@$DB_HOST:$DB_PORT/$DB_NAME"
# generic debugging options applicable to multiple commands
HOOT_OPTS="--info -D uuid.helper.repeatable=true -D writer.include.debug.tags=true"

OUTPUT_DIR=test-output/cmd/slow/$TEST_NAME
rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

if [ "$LOAD_REF_DATA" == "true" ]; then
  echo ""
  echo "STEP 1: Cleaning out the osm api db and initializing it for use..."
  echo ""
  scripts/database/CleanAndInitializeOsmApiDb.sh

  echo ""
  echo "STEP 2: Writing the complete reference dataset to the osm api db..."
  echo ""
  if [[ $REF_DATASET == *"pbf"* ]]; then
    #we want this in .osm format to be able to examine it for debugging purposes anyway
    hoot convert $HOOT_OPTS $REF_DATASET $OUTPUT_DIR/2-ref-raw-complete.osm
  else
    cp $REF_DATASET $OUTPUT_DIR/2-ref-raw-complete.osm
  fi 
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D changeset.user.id=1 -D osmapidb.bulk.writer.reserve.record.ids.before.writing.data=true -D osmapidb.bulk.writer.output.files.copy.location=$OUTPUT_DIR/2-ref-raw-complete.sql $OUTPUT_DIR/2-ref-raw-complete.osm $OSM_API_DB_URL
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 3: Reading the complete reference dataset out of the osm api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D writer.include.circular.error.tags=false $OSM_API_DB_URL $OUTPUT_DIR/3-ref-complete-PulledFromOsmApiDb.osm
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 4: Reading the subset AOI reference dataset out of the osm api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D convert.bounding.box=$AOI -D writer.include.circular.error.tags=false $OSM_API_DB_URL $OUTPUT_DIR/4-ref-subset-PulledFromOsmApiDb.osm
fi

if [ "$LOAD_SEC_DATA" == "true" ]; then
  echo ""
  echo "STEP 5: Writing the complete secondary dataset to the hoot api db..."
  echo ""
  if [[ $SEC_DATASET == *"pbf"* ]]; then
    #we want this in .osm format to be able to examine it for debugging purposes anyway
    hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true $SEC_DATASET $OUTPUT_DIR/5-secondary-raw-complete.osm
  else
    cp $SEC_DATASET $OUTPUT_DIR/5-secondary-raw-complete.osm
  fi
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D hootapi.db.writer.create.user=true -D hootapi.db.writer.overwrite.map=true $OUTPUT_DIR/5-secondary-raw-complete.osm "$HOOT_DB_URL/5-secondary-complete-$TEST_NAME"
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 6: Reading the complete secondary dataset out of the hoot api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D hootapi.db.writer.create.user=true -D hootapi.db.writer.overwrite.map=true "$HOOT_DB_URL/5-secondary-complete-$TEST_NAME" $OUTPUT_DIR/6-secondary-complete-PulledFromHootApiDb.osm
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 7: Reading the subset AOI secondary dataset out of the hoot api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D hootapi.db.writer.create.user=true -D hootapi.db.writer.overwrite.map=true -D convert.bounding.box=$AOI "$HOOT_DB_URL/5-secondary-complete-$TEST_NAME" $OUTPUT_DIR/7-secondary-subset-PulledFromHootApiDb.osm
fi

if [ "$CONFLATE_DATA" == "true" ]; then
  echo ""
  echo "STEP 8: Conflating the two datasets over the specified AOI..."
  echo ""
  # We're writing the output to the hoot api db first here, rather than directly to the osm api db, b/c if there are reviews 
  # we want to give the user a chance to review them.  That can only happen when the output is stored in a hoot api db.
  hoot conflate $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D hootapi.db.writer.create.user=true -D hootapi.db.writer.overwrite.map=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D convert.bounding.box=$AOI -D reader.conflate.use.data.source.ids=true -D preserve.unknown1.element.id.when.modifying.features=true -D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader -D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter -D osmapidb.id.aware.url=$OSM_API_DB_URL $OSM_API_DB_URL "$HOOT_DB_URL/5-secondary-complete-$TEST_NAME" "$HOOT_DB_URL/8-conflated-$TEST_NAME"
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 9: Reading the complete conflated dataset out of the hoot api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D reader.use.file.status=true -D reader.keep.file.status=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D writer.include.circular.error.tags=false "$HOOT_DB_URL/8-conflated-$TEST_NAME" $OUTPUT_DIR/9-conflated-complete-PulledFromHootApiDb.osm
fi

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 10: Reading the subset AOI conflated dataset out of the hoot api db and writing it into a file (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D reader.use.file.status=true -D reader.keep.file.status=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D writer.include.circular.error.tags=false "$HOOT_DB_URL/8-conflated-$TEST_NAME" $OUTPUT_DIR/10-conflated-subset-PulledFromHootApiDb.osm
fi

echo ""
echo "STEP 11: Writing a SQL changeset file that is the difference between the cropped reference input dataset specified AOI and the conflated output specified AOI..."
echo ""
hoot derive-changeset $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D reader.use.file.status=true -D reader.keep.file.status=true -D changeset.user.id=1 -D osm.changeset.sql.file.writer.generate.new.ids=false -D convert.bounding.box=$AOI -D changeset.buffer=0.001 -D changeset.allow.deleting.from.features=false -D changeset.compare.status=false $OSM_API_DB_URL "$HOOT_DB_URL/8-conflated-$TEST_NAME" $OUTPUT_DIR/11-conflated-changeset-ToBeAppliedToOsmApiDb.osc.sql $OSM_API_DB_URL
echo ""
echo "STEP 12: Writing an XML changeset file that is the difference between the cropped reference input dataset specified AOI and the conflated output specified AOI (will not be applied to the database)..."
echo ""
# changeset.xml.writer.add.timestamp should only be set false for this test's purposes; leave it set to the default value of
# true for production purposes
hoot derive-changeset $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D api.db.email=OsmApiDbHootApiDbConflate@hoottestcpp.org -D reader.use.file.status=true -D reader.keep.file.status=true -D changeset.user.id=1 -D osm.changeset.sql.file.writer.generate.new.ids=false -D convert.bounding.box=$AOI -D changeset.buffer=0.001 -D changeset.allow.deleting.from.features=false -D changeset.compare.status=false -D changeset.xml.writer.add.timestamp=false $OSM_API_DB_URL "$HOOT_DB_URL/8-conflated-$TEST_NAME" $OUTPUT_DIR/12-conflated-changeset-ToBeAppliedToOsmApiDb.osc

echo ""
echo "STEP 13: Executing the changeset SQL on the osm api db..."
echo ""
hoot apply-changeset $HOOT_OPTS $OUTPUT_DIR/11-conflated-changeset-ToBeAppliedToOsmApiDb.osc.sql $OSM_API_DB_URL

if [ "$RUN_DEBUG_STEPS" == "true" ]; then
  echo ""
  echo "STEP 14: Reading the contents of the osm api db for the specified aoi, writing it into a file, and verifying it (debug)..."
  echo ""
  hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D reader.use.file.status=true -D reader.keep.file.status=true -D writer.include.circular.error.tags=false -D convert.bounding.box=$AOI $OSM_API_DB_URL $OUTPUT_DIR/14-subset-output-PulledFromOsmApiDb.osm
fi

echo ""
echo "STEP 15: Reading the entire contents of the osm api db, writing it into a file, and verifying the data..."
echo ""
hoot convert $HOOT_OPTS -D reader.add.source.datetime=false -D reader.preserve.all.tags=true -D reader.use.file.status=true -D reader.keep.file.status=true -D writer.include.circular.error.tags=false $OSM_API_DB_URL $OUTPUT_DIR/15-complete-output-PulledFromOsmApiDb.osm
hoot is-match test-files/cmd/slow/$TEST_NAME/output.osm $OUTPUT_DIR/15-complete-output-PulledFromOsmApiDb.osm
echo ""
echo "STEP 16: Verifying the SQL changeset..."
echo ""
diff test-files/cmd/slow/$TEST_NAME/output.osc.sql $OUTPUT_DIR/11-conflated-changeset-ToBeAppliedToOsmApiDb.osc.sql
echo ""
echo "STEP 17: Verifying the XML changeset..."
echo ""
diff test-files/cmd/slow/$TEST_NAME/output.osc $OUTPUT_DIR/12-conflated-changeset-ToBeAppliedToOsmApiDb.osc


