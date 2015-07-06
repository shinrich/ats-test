import time,sys

if __name__ == "__main__":

	if len(sys.argv) < 2:
		print 'Must pass in file name'
		exit

	file = sys.argv[1]
	with open(file, "r") as f:
		expect_start = False
		count = 0
		begin = 0
		overtime = 0
		for line in f:
			if line == "start\n":
				expect_start = True
			elif expect_start == True:
				begin_time = time.strptime(line, "%a %b %d %H:%M:%S %Z %Y\n")
				begin = time.mktime(begin_time)
				expect_start = False
			
			else:
				end_time = time.strptime(line, "%a %b %d %H:%M:%S %Z %Y\n")
				end = time.mktime(end_time)
				diff = end - begin
				count = count + 1
                                if diff > 20:
					print 'Diff time is {0}'.format(diff)	
					overtime = overtime + 1

		print '{0} runs {1} overtime'.format(count, overtime)
