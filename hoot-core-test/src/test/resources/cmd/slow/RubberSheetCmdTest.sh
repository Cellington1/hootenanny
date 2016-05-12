#!/bin/bash
set -e

mkdir -p $HOOT_HOME/test-output/cmd/slow/RubberSheetCmdTest

input1=$HOOT_HOME/test-files/DcGisRoads.osm
input2=$HOOT_HOME/test-files/DcTigerRoads.osm
outputfile=$HOOT_HOME/test-output/cmd/slow/RubberSheetCmdTest/out.osm
comparefile=$HOOT_HOME/hoot-core-test/src/test/resources/cmd/slow/RubberSheetCmdTest/out.osm

# Run the command.
hoot rubber-sheet $input1 $input2 $outputfile | \
           sed "s/RMSE: [0-9]\+\.[0-9]\+//g"

# Check output
hoot is-match $comparefile $outputfile || diff $comparefile $outputfile

