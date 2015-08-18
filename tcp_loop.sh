echo "Start Test" >  output.txt
num=1
while [ $num -lt 10000 ]
do
  pnum=0
  while [ $pnum -lt 14 ] 
  do
    ./tcp-clients q1.ycpi.bf1.yahoo.net 50 3 &
    pnum=$(($pnum + 1))
  done
  sleep 0.5
  num=$(($num + 1))
done
wait
