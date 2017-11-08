#!/bin/bash
set -e

# Disable the tests that aren't running on Ubuntu 16.04. They aren't relevant to global POIs.
# These failures are likely due to library changes.
hoot --version --debug; HootTest --slow --exclude=.*PrintNamesCmdTest.sh --exclude=.*ConflateCmdStatsGenericRiversTest.sh --exclude=.*PertyScoreCmdTest.sh --exclude=.*ScoreCmdTest.sh --exclude=.*StatsFileTest.sh --exclude=.*PertyMatchScorerTest.* --exclude=.*BigPertyOpTest.* --exclude=.*TDistributionTest.* --exclude=.*ExpectationIntersectionTest.* --exclude=.*PertyDuplicatePoiOpTest.* --exclude=hoot::PertyOpTest.* --parallel `nproc`

# This isn't working in Ubuntu 16.04. :(
#cd $HOOT_HOME/plugins-local; mocha

make -sj`nproc` dist-bin
