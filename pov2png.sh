filenames=$(ls *.pov)
for f in ${filenames};do
	~/Downloads/povray-3.6/povray -H2160 -W2160 ${f}
done
