1. number of vertexes: nnum
2. graph.bin: 
	There are nnum lines and for each line, the format is:
	v ngt nlt u1 u2 ...(ngt adjacent vertexes whose order is greater than v) w1 w2 ...(nlt adjacent vertexes whose order is less than v

3. order.bin
	There are nnum lines and for each line, the format is 
	vto v (the vertex number of vertex v is vto)

Example:
graph.bin: 2 4 2 3 4 5 6 0 1
For vertex 2, it has 6 adjacent vertexes. And among them, there are 4 adjacent vertexes (3,4,5,6) whose order is greater than 2, and 2 adjacent vertexes (0,1) whose order is smaller than 2.
order.bin: v v
Suppose for vertex v, its order is v.

For HDFS version, it shall partition graph.bin into MAX_B blocks, and put them int the HDFS InDir firstly.
  
