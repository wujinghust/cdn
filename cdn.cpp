/*
 * cdn.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: wujing
 */

#include "deploy.h"
#include "lib/lib_io.h"
#include "lib/lib_time.h"
#include "stdio.h"

int main(int argc, char *argv[])
{
	print_time("Begin");
	char *topo[MAX_EDGE_NUM];
	int line_num;

	//char *topo_file = argv[1];
	char topo_file[128] = "case1.txt";
	line_num = read_file(topo, MAX_EDGE_NUM, topo_file);

	if (line_num == 0)
	{
		printf("Please input valid topo file.\n");
		return -1;
	}

	//char *result_file = argv[2];
	char result_file[128] = "result1.txt";

	deploy_server(topo, line_num, result_file);

	release_buff(topo, line_num);

	print_time("End");

	return 0;
}




