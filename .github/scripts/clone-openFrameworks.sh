
pushd Libraries
git clone --recursive --shallow-submodules https://github.com/openframeworks/openFrameworks.git
pushd openFrameworks
git reset --hard ac69b2f
./scripts/ci/vs/install.sh
popd
popd
