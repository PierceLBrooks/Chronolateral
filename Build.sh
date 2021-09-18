git submodule update --init
python ./Buildster/buildster.py ./Project.xml
chmod +x ./Distribution/debug/*
patchelf --set-rpath '$ORIGIN' ./Distribution/debug/objsheet
