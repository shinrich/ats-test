echo "Start Test" >  output.txt
num=1
while [ $num -lt 100 ]
do
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  sleep 0.5
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  ./tcp-clients draggingnagging.corp.ne1.yahoo.com 50 3 &
  sleep 0.5
  num=$(($num + 1))
done
wait
