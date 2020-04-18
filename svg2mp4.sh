cd output
python ../PhysiCell_tools/svg2other.py 0 120 2 70 jpg
mkdir movie
mv *.jpg movie
cd movie/
convert -delay 1x4 *jpg tmp.mp4
ffmpeg -i tmp.mp4 -vf scale=1080:-2,format=yuv420p movie.mp4 
rm tmp.mp4
#ffplay movie.mp4

