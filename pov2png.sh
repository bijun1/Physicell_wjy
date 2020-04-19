if [ ! -d "/pov_file/" ];then
mkdir /pov_file
fi

if [ ! -d "/png_file/" ];then
mkdir /png_file
fi

mv *.pov pov_file

filenames=$(ls pov_file/*.pov)
res=2000;
for f in ${filenames};do
	~/Downloads/povray-3.6/povray -H${res} -W${res} ${f}
done

mv *.png png_file/

mv pov_file pov_file_done
