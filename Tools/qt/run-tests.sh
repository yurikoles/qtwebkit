#!/bin/sh
export WEBKIT_OUTPUTDIR=$(pwd)/WebKitBuild-Docker
export PATH=${WEBKIT_OUTPUTDIR}/DependenciesQT/Root/bin:$PATH
set -e
git clone https://github.com/qtwebkit/test-fonts.git
export WEBKIT_TESTFONTS=$(pwd)/test-fonts
ln -sf $(pwd) /var/tmp/qtwebkit-build
Tools/Scripts/run-webkit-tests --qt -1 \
        --no-build --clobber-old-results \
        --no-show-results --no-new-test-results \
        --child-processes=2 --results-directory=$(pwd)/layout-test-results fast
tar zcf layout-tests.tar.gz $(pwd)/layout-test-results
