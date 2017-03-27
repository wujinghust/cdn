#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stack>
#include<iostream>
#include<time.h>


using namespace std;
#ifdef _DEBUG
#define PRINT   printf
#else
#define PRINT(...)
#endif
//////////////////////////////////////////////
typedef struct _Edge
{
	int bandwidth;
	int cost;
}Edge;
typedef struct _ConsumeNode
{
	int id;
	int link_id;
	int demand;
	bool fulfill;
}ConsumeNode;

int Dijkstra(Edge*** matrix, ConsumeNode* consume_node, stack<int>& path, int source, int dest);
void Initialize(Edge*** matrix, int line_num);
int LoadFromFile(Edge*** matrix, ConsumeNode* consume_node, char** topo, int line_num);
int UpdateMatrix(Edge*** matrix, const stack<int>& path, ConsumeNode* consume_node);
void ShowPath(stack<int> path, int consume_id, int flow, bool fulfill);


//你要完成的功能总入口
int netnode_num, netlink_num, consumenode_num;
const int INF = 100001;
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{

	int server_cost;
	char buf[128];
	strcpy(buf, topo[0]);
	sscanf(buf, "%d %d %d\n", &netnode_num, &netlink_num, &consumenode_num);

	memset(buf, '\0', 128);

	strcpy(buf, topo[2]);
	sscanf(buf, "%d\n", &server_cost);

	
	//printf("test server_cost= %d\n", server_cost);

	Edge*** matrix = (Edge***)malloc(netnode_num * sizeof(Edge**));
	if (matrix == NULL)
	{
		PRINT("malloc matrix failed :%s", strerror(errno));
		exit(1);
	}
	for (int i = 0; i < netnode_num; ++i)
	{
		matrix[i] = (Edge**)malloc(netnode_num * sizeof(Edge*));
		if (matrix[i] == NULL)
		{
			PRINT("malloc matrix failed: %s", strerror(errno));
			exit(1);
		}
	}
	ConsumeNode* consume_node = (ConsumeNode*)malloc(consumenode_num * sizeof(ConsumeNode));

	Initialize(matrix, netnode_num);
	LoadFromFile(matrix, consume_node, topo, line_num);
	int total_cost = 0;
	int index = 0;
	srand((unsigned int)time(NULL));
	int begin = 0;
	int* dis = new int[consumenode_num];
	memset(dis, 0, consumenode_num * sizeof(int));

	while (true)
	{
		int min =INF;
		if (consume_node[index].fulfill == false)
		{
			stack<int> path;
			
			for (int i = 0; i < consumenode_num; i++)
			{
				dis[i] = Dijkstra(matrix, consume_node, path, begin, consume_node[i].link_id);
				if (dis[i] < min)
				{
					min = dis[i];
					index = i;
				}
				path.size();
			}

			int flow = UpdateMatrix(matrix, path, &consume_node[index]);
			ShowPath(path, consume_node[index].id, flow, consume_node[index].fulfill);
						
			total_cost += min*flow;
		}

		bool flag = true;

		for (int i = 0; i < consumenode_num; i++)
		{
			if (consume_node[i].fulfill == false)
				flag = false;
		}
		if (flag == true)
			break;
		index++;
		if (index == consumenode_num)
		{
			index = 0;
			
		}
		begin = rand() % (netnode_num);

	}
	cout << "total cost:" << total_cost << endl;
	// 需要输出的内容
	//char * topo_file = (char *)"17\n\n0 8 0 20\n21 8 0 20\n9 11 1 13\n21 22 2 20\n23 22 2 8\n1 3 3 11\n24 3 3 17\n27 3 3 26\n24 3 3 10\n18 17 4 11\n1 19 5 26\n1 16 6 15\n15 13 7 13\n4 5 8 18\n2 25 9 15\n0 7 10 10\n23 24 11 23";
	char* topo_file = (char*)malloc(consumenode_num*1024 * sizeof(char));
	memset(topo_file, 0, consumenode_num*1024);

	if (topo_file == NULL)
	{
		PRINT("topo_file malloc failed %s\n", strerror(errno));
		exit(1);
	}
	char buffer[128];
	sprintf(buffer, "%d\n\n", consumenode_num);
	printf(buffer, "%d\n\n", consumenode_num);
	strcat(topo_file, buffer);

	for (int i = 0; i < consumenode_num; ++i)
	{
		char buf[128];
		sprintf(buf, "%d %d %d\n", consume_node[i].link_id, consume_node[i].id, consume_node[i].demand);
		printf(buf, "%d %d %d\n", consume_node[i].link_id, consume_node[i].id, consume_node[i].demand);
		strcat(topo_file, buf);
	}
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);


	for (int i = 0; i < netnode_num; i++)
	{
		for (int j = 0; j < netnode_num; j++)
		{
			if (matrix[i][j] != NULL)
				free(matrix[i][j]);
		}
	}

	for (int i = 0; i < netnode_num; i++)
	{
		free(matrix[i]);
	}
	free(matrix);

	free(consume_node);

	free(topo_file);
}
void Initialize(Edge*** matrix, int netnode_num)
{
	for (int i = 0; i < netnode_num; ++i)
	{
		for (int j = 0; j < netnode_num; ++j)
		{
			matrix[i][j] = NULL;
		}
	}
}
int LoadFromFile(Edge*** matrix, ConsumeNode* consume_node, char** topo, int line_num)
{
	//char** p = topo;
	int node_begin, node_end, bandwidth, cost;
	int id, link_id, demand;
	int index = 0;
	int flag = 0;
	for (int i = 4; i < line_num; ++i)
	{
		if (topo[i][0] == '\n')
		{
			flag = 1;
			continue;
		}
		if (flag == 0)
		{
			sscanf(topo[i], "%d %d %d %d\n", &node_begin, &node_end, &bandwidth, &cost);
			//printf("%d %d %d %d\n", node_begin, node_end, bandwidth, cost);

			if (matrix[node_begin][node_end] == NULL)
			{
				matrix[node_begin][node_end] = (Edge*)malloc(sizeof(Edge));
				matrix[node_end][node_begin] = (Edge*)malloc(sizeof(Edge));
				if (matrix[node_begin][node_end] == NULL)
				{
					PRINT("matirx[%d][%d] malloc failed :%s", node_begin, node_end, strerror(errno));
					exit(1);
				}
				if (matrix[node_end][node_begin] == NULL)
				{
					PRINT("matirx[%d][%d] malloc failed :%s", node_end, node_begin, strerror(errno));
					exit(1);
				}
			}
			matrix[node_begin][node_end]->bandwidth = bandwidth;
			matrix[node_begin][node_end]->cost = cost;

			matrix[node_end][node_begin]->bandwidth = bandwidth;
			matrix[node_end][node_begin]->cost = cost;

		}
		else
		{
			int ret;

			if ((ret = sscanf(topo[i], "%d %d %d\n", &id, &link_id, &demand)) != 3)
			{
				PRINT("sscanf consume node failed :%s", strerror(errno));
				exit(1);
			}
			consume_node[index].id = id;
			consume_node[index].link_id = link_id;
			consume_node[index].demand = demand;
			consume_node[index].fulfill = false;

			printf("%d %d %d \n", consume_node[index].id = id, consume_node[index].link_id = link_id, consume_node[index].demand = demand);
			index++;
		}
	}

	return 0;
}
int Dijkstra(Edge*** matrix, ConsumeNode* consume_node, stack<int>& path, int source, int dest)
{
	while (!path.empty())
	{
		path.pop();
	}
	if (source == dest)
	{
		path.push(source);
		return 0;
	}
	bool* vis = new bool[netnode_num];//相当于集合Q的功能， 标记该点是否访问过
	memset(vis, 0, netnode_num * sizeof(bool));
	int* dis = new int[netnode_num];//保存最短路径
	int* prev = new int[netnode_num];

	int i, j, k;

	for (i = 0; i < netnode_num; i++)//初始化
	{
		if (matrix[source][i] != NULL&&matrix[source][i]->bandwidth != 0)
		{
			dis[i] = matrix[source][i]->cost;//s—>各个点的距离
			prev[i] = source;

		}
		else
		{
			dis[i] = INF;
			prev[i] = -1;

		}
	}
	prev[source] = -2;
	dis[source] = 0;//s->s 距离为0
	vis[source] = true;//s点访问过了，标记为真

	for (i = 0; i < netnode_num; i++)//G.V-1次操作+上面对s的访问 = G.V次操作
	{
		k = source;
		for (j = 0; j < netnode_num; j++)//从尚未访问过的点中选一个距离最小的点
		{
			if (!vis[j] && (k == source || dis[k] > dis[j]))//未访问过 && 是距离最小的
				k = j;
		}
		if (k == source)//若图是不连通的则提前结束
			break;//跳出循环
		vis[k] = true;//将k点标记为访问过了
		bool avail = false;
		for (int i = 0; i < netnode_num; i++)
		{
			if (matrix[k][i] != NULL&&matrix[k][i]->cost > 0)
			{
				avail = true;
				break;
			}
		}
		if (avail == false)
		{
			return INF;
		}
		for (j = 0; j < netnode_num; j++)//松弛操作
		{

			if (!vis[j] && matrix[k][j] != NULL&&matrix[k][j] > 0 && dis[j] > dis[k] + matrix[k][j]->cost)//该点为访问过 && 可以进行松弛
			{
				dis[j] = dis[k] + matrix[k][j]->cost;//j点的距离  大于当前点的距离+w(k,j) 则松弛成功，进行更新
				prev[j] = k;
			}
		}
		if (k == dest)
			break;
	}

	int temp = dest;
	while (temp != source)
	{
		path.push(temp);
		temp = prev[temp];
	}
	path.push(temp);


	int res = dis[dest];

	delete[] vis;
	delete[] prev;
	delete[] dis;

	return res;
}
int UpdateMatrix(Edge*** matrix,const stack<int>& path, ConsumeNode* consume_node)
{
	if (path.size() == 1)
	{
		consume_node->fulfill = true;
		return 0;
	}
	int supply = INF;
	int begin, end;
	stack<int> cpath(path);

	begin = cpath.top();
	if (begin<0 || begin>netnode_num)
	{
		cerr << "error begin" << begin << endl;;
	}
	while (cpath.size() > 1)
	{
		cpath.pop();
		end = cpath.top();
		if (end<0 || end>netnode_num)
		{
			cerr << "error end" << end << endl;;
		}
		if (matrix[begin][end]->bandwidth < supply)
			supply = matrix[begin][end]->bandwidth;
		begin = end;
	}
	if (supply == 0)
		return 0;

	cpath = path;
	begin = cpath.top();

	if (consume_node->demand <= supply)
	{
		while (cpath.size() > 1)
		{
			cpath.pop();
			end = cpath.top();
			matrix[begin][end]->bandwidth -= consume_node->demand;

			begin = end;
		}

		consume_node->fulfill = true;
		return consume_node->demand;
	}
	else
	{
		while (cpath.size() > 1)
		{
			cpath.pop();
			end = cpath.top();
			matrix[begin][end]->bandwidth = 0;
			begin = end;
		}
		consume_node->demand -= supply;
		return supply;
	}
}
void ShowPath(stack<int> path, int consume_id, int flow, bool fulfill)
{
	
	while (!path.empty())
	{
		cout << path.top() << " ";
		path.pop();
	}
	string full;
	if (fulfill)
		full = "fulfill";
	else
		full = "no fulfill";
	cout << consume_id << " " << flow << " " << fulfill << endl;
}