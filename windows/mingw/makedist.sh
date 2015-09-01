if [[ ! -f "./moria.exe" ]]; then
	echo "ERROR: ./moria.exe does not exist (still needs to be built?)"
	exit 1
fi

mkdir -p ./dist/doc
echo === Copying Windows files ===
COPY="cp -f --preserve=all"
${COPY} moria.exe ./dist/
${COPY} ../MORIA.CNF ./dist/
${COPY} readme.txt ./dist/
echo === Copying data files ===
${COPY} ../../files/* ./dist/
echo === Copying documentation files ===
${COPY} ../../doc/* ./dist/doc/
echo ...Setup complete.
