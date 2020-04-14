cd output
python ../PhysiCell_tools/svg2other.py 0 120 2 70 jpg
mkdir movie
mv *.jpg movie
cd movie/
convert -delay 1x4 *jpg movie.mp4
ffplay movie.mp4

