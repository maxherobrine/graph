#include <fstream>
#include <iostream>
#include <clocale>
#include <algorithm>
#include <vector>
#include <queue>
#include <stack>
using namespace std;

#define OUTPUT 1
#define COUNT 0
#define CONTRACTION 1
#define IDENTIFICATION 0
#define REPRODUCTION 1
#define DUPLICATION 0
#define CONNECT 1
#define UNION 0

class Graph {
private:
	bool OrGraph = 0; //0 - не ориентированный граф; 1 - орграф
	bool GraphView[3] = { 0 };	//Индексы: 0 - список смежности; 1 - матрица смежности; 2 - матрица инцидентности. Значения: 0 - граф не задан данным способом; 1 - задан.
	int AdjacencyList[100][100] = { 0 };
	int AdjacencyMatrix[100][100] = { 0 };
	int IncidenceMatrix[100][100] = { 0 };
	int WeightMatrix[100][100] = { 0 };
	unsigned int NameVertex[100] = { 0 };
	unsigned int NumEdges = 0;	//Количество столбцов в матрице инцидентности.
	unsigned int NumVertex = 0;	//Размерность матрицы смежности и массива NumAdjacentVert; количество строк в матрице инцидентности.
	unsigned int NumAdjacentVert[100] = { 0 };	//Количество смежных вершин для i-ой вершины (размерность списка смежности). == Степенной последовательности и полустепени исхода
	unsigned int InDegree[100] = { 0 };	//Полустепень захода (не сортирован, по номеру вершины можно узнать её ПЗ)
	unsigned int Path[100] = { 0 }, bufPath[100] = { 0 };	//Путь до вершины
	unsigned int Distance = 101, bufDistance = 0;	//Расстояние между вершинами
	unsigned int Eccentricitys[100] = { 0 };
	unsigned int Radius = 101;
	unsigned int Diameter = 0;
	unsigned int SourceSink[100] = { 0 };

	void TranslationToOtherRepresent() {
		if (GraphView[0]) {
			memset(AdjacencyMatrix, 0, sizeof(int) * 10000);
			memset(IncidenceMatrix, 0, sizeof(int) * 10000);
			for (int i = 0;i < NumVertex;i++)	//Из СС в МС
				for (int j = 0;AdjacencyList[i][j] != 0;j++)
					AdjacencyMatrix[i][AdjacencyList[i][j] - 1]++;
			int edge = 0;
			for (int i = 0;i < NumVertex;i++)	//Из СС в МИ
				for (int j = 0;AdjacencyList[i][j] != 0;j++) {
					if (AdjacencyList[i][j] == (i + 1))
					{
						IncidenceMatrix[i][edge] = 2;
						InDegree[i]++;
						edge++;
						continue;
					}
					if (OrGraph || (AdjacencyList[i][j] > (i + 1)))
					{
						IncidenceMatrix[i][edge] = 1;
						if (!OrGraph)
							IncidenceMatrix[AdjacencyList[i][j] - 1][edge] = 1;
						else {
							IncidenceMatrix[AdjacencyList[i][j] - 1][edge] = -1;
							InDegree[AdjacencyList[i][j] - 1]++;
						}
						edge++;
					}
				}
			NumEdges = edge;
			return;
		}
		if (GraphView[1]) {
			memset(AdjacencyList, 0, 10000);
			memset(IncidenceMatrix, 0, 10000);
			unsigned int num_ajac_v = 0;
			for (int i = 0;i < NumVertex;i++) {	//Из МС в СС
				for (int j = 0;j < NumVertex;j++)
					for (int k = 0;k < AdjacencyMatrix[i][j];k++) {
						AdjacencyList[i][num_ajac_v] = NameVertex[j];
						num_ajac_v++;
					}
				NumAdjacentVert[i] = num_ajac_v;
				num_ajac_v = 0;
			}
			int edge = 0;
			for (int i = 0;i < NumVertex;i++)	//Из СС в МИ
				for (int j = 0;AdjacencyList[i][j] != 0;j++) {
					if (AdjacencyList[i][j] == NameVertex[i])
					{
						IncidenceMatrix[i][edge] = 2;
						InDegree[i]++;
						edge++;
						continue;
					}
					if (OrGraph || (AdjacencyList[i][j] > NameVertex[i]))
					{
						IncidenceMatrix[i][edge] = 1;
						int l = 0;
						for (;l < NumVertex;l++)
							if (AdjacencyList[i][j] == NameVertex[l])
								break;
						if (!OrGraph)
							IncidenceMatrix[l][edge] = 1;
						else {
							IncidenceMatrix[l][edge] = -1;
							InDegree[l]++;
						}
						edge++;
					}
				}
			NumEdges = edge;
			return;
		}
		if (GraphView[2]) {
			memset(AdjacencyList, 0, 10000);
			memset(AdjacencyMatrix, 0, 10000);
			unsigned int num_ajac_v = 0;
			for (int i = 0;i < NumVertex;i++) {	//Из МИ в СС
				for (int j = 0;j < NumEdges;j++) {
					if (IncidenceMatrix[i][j] == 2) {
						AdjacencyList[i][num_ajac_v] = NameVertex[i];
						num_ajac_v++;
						InDegree[i]++;
					}
					if (IncidenceMatrix[i][j] == 1) {
						int k = 0;
						do {
							if (k == i)
								k++;
							else if (IncidenceMatrix[k][j] == 0)
								k++;
							else
								break;
						} while (k < NumVertex);
						AdjacencyList[i][num_ajac_v] = NameVertex[k];
						num_ajac_v++;
					}
					if (IncidenceMatrix[i][j] == -1)
						InDegree[i]++;
				}
				NumAdjacentVert[i] = num_ajac_v;
				num_ajac_v = 0;
			}
			for (int i = 0;i < NumVertex;i++)	//Из СС в МС
				for (int j = 0;AdjacencyList[i][j] != 0;j++)
					AdjacencyMatrix[i][AdjacencyList[i][j] - 1]++;
			return;
		}
	}

public:
	//Конструкторы
	Graph() {}
	Graph(bool type, int arr[100][100], unsigned int vert, unsigned int adjvert[100]) {	//Список смежности
		OrGraph = type;
		GraphView[0] = 1;
		NumVertex = vert;
		for (int i = 0;i < NumVertex;i++)
			NameVertex[i] = i + 1;
		memcpy(NumAdjacentVert, adjvert, sizeof(int)*vert);
		for (int i = 0;i < vert;i++)
			memcpy(AdjacencyList[i], arr[i], sizeof(int)*adjvert[i]);
		TranslationToOtherRepresent();
	}
	Graph(bool type, int arr[100][100], unsigned int vert) {	//Матрица смежности
		OrGraph = type;
		GraphView[1] = 1;
		NumVertex = vert;
		for (int i = 0;i < NumVertex;i++)
			NameVertex[i] = i + 1;
		memcpy(AdjacencyMatrix, arr, sizeof(int) * 10000);
		TranslationToOtherRepresent();
	}
	Graph(bool type, int arr[100][100], unsigned int vert, unsigned int edge) {	//Матрица инцидентности
		OrGraph = type;
		GraphView[2] = 1;
		NumVertex = vert;
		for (int i = 0;i < NumVertex;i++)
			NameVertex[i] = i + 1;
		NumEdges = edge;
		memcpy(IncidenceMatrix, arr, sizeof(int) * 10000);
		TranslationToOtherRepresent();
	}

	//Методы
	void getAdjacencyList() {	//Вывод списка смежности
		cout << "Представление графа в виде списка смежности:" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << "{ " << NameVertex[i] << ": ";
			for (int j = 0;AdjacencyList[i][j] != 0;j++)
				cout << AdjacencyList[i][j] << " ";
			cout << "}" << endl;
		}
		cout << endl;
	}
	void getAdjacencyMatrix() {	//Вывод матрицы смежности
		cout << "Представление графа в виде матрицы смежности:" << endl;
		cout << "/--";
		for (int i = 0;i < NumVertex;i++)
			cout << NameVertex[i] << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i] << "  ";
			for (int j = 0;j < NumVertex;j++)
				cout << AdjacencyMatrix[i][j] << " ";
			cout << endl;
		}
		cout << endl;
	}
	void getIncidenceMatrix() {	//Вывод матрицы инцидентности
		cout << "Представление графа в виде матрицы инцидентности:" << endl;
		cout << "/--";
		if (OrGraph)
			for (int i = 0;i < NumEdges;i++)
				cout << (i + 1) << "--";
		else
			for (int i = 0;i < NumEdges;i++)
				cout << (i + 1) << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i];
			if (!OrGraph)
				cout << " ";
			for (int j = 0;j < NumEdges;j++) {
				if (OrGraph && j > 8)
					cout.width(4);
				else if (OrGraph || j > 8)
					cout.width(3);
				else
					cout.width(2);
				cout << IncidenceMatrix[i][j];
			}
			cout << endl;
		}
		cout << endl;
	}
	void getInfo() {
		cout << "Число вершин: " << NumVertex << endl;
		cout << "Число рёбер(дуг): " << NumEdges << endl;
		vector<int> sort_int_vector(NumAdjacentVert, NumAdjacentVert + NumVertex);
		sort(sort_int_vector.begin(), sort_int_vector.end());
		if (!OrGraph)
			cout << "Степенная последовательность: ";
		else
			cout << "Полустепень исхода: ";
		for (int i = NumVertex - 1;i > 0;i--)
			cout << sort_int_vector[i] << ", ";
		cout << sort_int_vector[0] << "." << endl;
		if (OrGraph) {
			vector<int> sort_int_vector(InDegree, InDegree + NumVertex);
			sort(sort_int_vector.begin(), sort_int_vector.end());
			cout << "Полустепень захода: ";
			for (int i = NumVertex - 1;i > 0;i--)

				cout << sort_int_vector[i] << ", ";
			cout << sort_int_vector[0] << "." << endl;
		}
		cout << endl;
	}
	void getPowerVertex(unsigned int v) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Вершины " << v << " не существует!" << endl << endl;
			return;
		}
		if (!OrGraph)
			cout << "Степень вершины " << v << ": ";
		else
			cout << "Полустепень исхода вершины " << v << ": ";
		cout << NumAdjacentVert[i] << "." << endl;
		if (OrGraph)
			cout << "Полустепень захода: " << InDegree[i] << "." << endl;
		cout << endl;
	}
	void getHengingIsolatedVert() {
		cout << "Висячие вершины: ";
		for (int i = 0;i < NumVertex;i++)
			if (NumAdjacentVert[i] == 1)
				cout << NameVertex[i] << "; ";
		cout << endl;
		cout << "Изолированные вершины: ";
		for (int i = 0;i < NumVertex;i++)
			if (NumAdjacentVert[i] == 0)
				cout << NameVertex[i] << "; ";
		cout << endl << endl;
	}
	//Стоки, истоки орграфа
	unsigned int getSourceSink() {
		if (!OrGraph) {
			cout << "Граф не ориентированный!";
			return 1;
		}
		unsigned int num_ss = 0;
		cout << "Истоки: ";
		for (int i = 0;i < NumVertex;i++) {
			int j = 0;
			for (j = 0;j < NumEdges;j++)
				if (IncidenceMatrix[i][j] == -1)
					break;
			if (j == NumEdges) {
				cout << NameVertex[i] << "; ";
				SourceSink[num_ss] = i;
				num_ss++;
			}
		}
		cout << endl;
		cout << "Стоки: ";
		for (int i = 0;i < NumVertex;i++) {
			int j = 0;
			for (j = 0;j < NumEdges;j++)
				if (IncidenceMatrix[i][j] == 1)
					break;
			if (j == NumEdges) {
				cout << NameVertex[i] << "; ";
				SourceSink[num_ss] = i;
				num_ss++;
			}
		}
		cout << endl << endl;
		return num_ss;
	} 
	int getDistance(unsigned int v1, unsigned int v2, bool out) {
		int l = 0;
		for (;l < NumVertex;l++)
			if (NameVertex[l] == v1)
				break;
		if (l == NumVertex) {
			cout << "Вершины " << v1 << " не существует!" << endl << endl;
			return -1;
		}
		if (bufPath[0] == 0) {
			int i = 0;
			for (;i < NumVertex;i++)
				if (NameVertex[i] == v2)
					break;
			if (i == NumVertex) {
				cout << "Вершины " << v2 << " не существует!" << endl << endl;
				return -1;
			}
			if ((!OrGraph && (AdjacencyList[l][0] == 0 || AdjacencyList[i][0] == 0)) || (OrGraph&&AdjacencyList[l][0] == 0) || v1 == v2) {
				if (out)
					cout << "Одна из вершин - изолированная, либо вы ввели одну вершину!" << endl << endl;
				return -1;
			}
			Distance = 101;
		}
		bufPath[bufDistance] = v1;
		bufDistance++;
		if (bufDistance >= Distance || AdjacencyList[l][0] == 0) //Если путь уже больше пройденного ранее или пути дальше нет - выходим
			return 1;
		for (int j = 0;AdjacencyList[l][j] != 0;j++)
			if (AdjacencyList[l][j] == v2) {
				Distance = bufDistance;
				memcpy(Path, bufPath, sizeof(unsigned int)*Distance);
				if (Distance == 1) {
					if (out) {
						cout << "Расстояние: 1." << endl;
						cout << "Путь: " << v1 << " -> " << v2 << endl << endl;
					}
					bufDistance = 0;
					memset(bufPath, 0, sizeof(unsigned int) * 100);
					memset(Path, 0, sizeof(unsigned int) * 100);
					return 0;
				}
				return 1;
			}
		for (int j = 0;AdjacencyList[l][j] != 0;j++) {
			int k = 0;
			for (;k < bufDistance;k++)
				if (AdjacencyList[l][j] == bufPath[k])
					break;
			if (k != bufDistance)
				continue;
			if (getDistance(AdjacencyList[l][j], v2, out) == 1) {
				bufDistance--;
				bufPath[bufDistance] = 0;
			}
			else
				return 0;
		}
		if (v1 == bufPath[0]) {
			if (Distance < 101) {
				if (out)
					cout << "Расстояние: " << Distance << endl;
				Path[Distance] = v2;
				if (out) {
					cout << "Путь: ";
					for (int k = 0;k < Distance;k++)
						cout << Path[k] << " -> ";
					cout << v2 << "." << endl << endl;
				}
				bufDistance = 0;
				memset(bufPath, 0, sizeof(unsigned int) * 100);
				memset(Path, 0, sizeof(unsigned int) * 100);
				return 0;
			}
			bufDistance = 0;
			memset(bufPath, 0, sizeof(unsigned int) * 100);
			memset(Path, 0, sizeof(unsigned int) * 100);
		}
		return 1;
	}
	void getEccentricity(unsigned int v, bool out) {
		int l = 0;
		for (;l < NumVertex;l++)
			if (NameVertex[l] == v)
				break;
		if (l == NumVertex) {
			cout << "Вершины " << v << " не существует!" << endl << endl;
			return;
		}
		if (Eccentricitys[l] == 0) {
			unsigned int ecc = 0;
			for (int i = 0;i < NumVertex;i++) {
				unsigned int v_end = NameVertex[i];
				if (getDistance(v, v_end, COUNT) == 0 && ecc < Distance)
					ecc = Distance;
			}
			Eccentricitys[l] = ecc;
		}
		if (out)
			cout << "Эксцентриситет вершины " << v << ": " << Eccentricitys[l] << "." << endl << endl;
	}
	void getRadius(bool out) {
		for (int i = 0;i < NumVertex;i++)
			getEccentricity(NameVertex[i], COUNT);
		for (int i = 0;i < NumVertex;i++)
			if (Eccentricitys[i] < Radius)
				Radius = Eccentricitys[i];
		if (out)
			cout << "Радиус графа: " << Radius << "." << endl << endl;
	}
	void getDiameter(bool out) {
		for (int i = 0;i < NumVertex;i++)
			getEccentricity(NameVertex[i], COUNT);
		for (int i = 0;i < NumVertex;i++)
			if (Eccentricitys[i] > Diameter)
				Diameter = Eccentricitys[i];
		if (out)
			cout << "Диаметр графа: " << Diameter << "." << endl << endl;
	}
	void getCenter() {
		if (Radius == 101)
			getRadius(COUNT);
		cout << "Центр(ы) графа: ";
		for (int i = 0;i < NumVertex;i++)
			if (Eccentricitys[i] == Radius)
				cout << NameVertex[i] << "; ";
		cout << endl << endl;
	}
	void getPeripheralVertices() {
		if (Diameter == 0)
			getDiameter(COUNT);
		cout << "Диаметр(ы) графа: ";
		for (int i = 0;i < NumVertex;i++)
			if (Eccentricitys[i] == Diameter)
				cout << NameVertex[i] << "; ";
		cout << endl << endl;
	}
	void addVertex(unsigned int v) {
		for (int i = 0;i < NumVertex;i++)
			if (NameVertex[i] == v) {
				cout << "Такая вершина уже существует!" << endl << endl;
				return;
			}
		NameVertex[NumVertex] = v;
		NumVertex++;
	}
	void delVertex(unsigned int v) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		for (int j = i;j < NumVertex - 1;j++) {
			memcpy(AdjacencyMatrix[j], AdjacencyMatrix[j + 1], sizeof(int) * 100);
			NameVertex[j] = NameVertex[j + 1];
		}
		memset(AdjacencyMatrix[NumVertex - 1], 0, sizeof(int) * 100);
		for (int j = 0;j < NumVertex - 1;j++) {
			for (int k = i;k < NumVertex - 1;k++)
				AdjacencyMatrix[j][k] = AdjacencyMatrix[j][k + 1];
			AdjacencyMatrix[j][NumVertex - 1] = 0;
		}
		NameVertex[NumVertex - 1] = 0;
		NumVertex--;
		GraphView[0] = 0;
		GraphView[1] = 1;
		GraphView[2] = 0;
		TranslationToOtherRepresent();
	}
	void addEdge(unsigned int v1, unsigned int v2) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v1)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int j = 0;
		for (;j < NumVertex;j++)
			if (NameVertex[j] == v2)
				break;
		if (j == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		AdjacencyMatrix[i][j]++;
		if (!OrGraph)
			AdjacencyMatrix[j][i]++;
		GraphView[0] = 0;
		GraphView[1] = 1;
		GraphView[2] = 0;
		TranslationToOtherRepresent();
	}
	void delEdge(unsigned int v1, unsigned int v2) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v1)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int j = 0;
		for (;j < NumVertex;j++)
			if (NameVertex[j] == v2)
				break;
		if (j == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		if (AdjacencyMatrix[i][j] == 0)
			cout << "Такого ребра не существует!" << endl << endl;
		else {
			AdjacencyMatrix[i][j]--;
			if (!OrGraph)
				AdjacencyMatrix[j][i]--;
			GraphView[0] = 0;
			GraphView[1] = 1;
			GraphView[2] = 0;
			TranslationToOtherRepresent();
		}
	}
	void findComplementGraph() {
		cout << "Матрица смежности дополнения графа:" << endl;
		cout << "/--";
		for (int i = 0;i < NumVertex;i++)
			cout << NameVertex[i] << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i] << "  ";
			for (int j = 0;j < NumVertex;j++)
				cout << (AdjacencyMatrix[i][j] + 1) % 2 << " ";
			cout << endl;
		}
		cout << endl;
	}
	void subdividGraphEdge(unsigned int v1, unsigned int v2) {
		delEdge(v1, v2);
		unsigned int v3 = NameVertex[NumVertex - 1] + 1;
		addVertex(v3);
		addEdge(v1, v3);
		addEdge(v3, v2);
	}
	void contractionEdge_identVertex(unsigned int v1, unsigned int v2, bool contract) {
		delEdge(v1, v2);
		unsigned int v3 = NameVertex[NumVertex - 1] + 1;
		addVertex(v3);
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v1)
				break;
		int j = 0;
		for (;j < NumVertex;j++)
			if (NameVertex[j] == v2)
				break;
		for (;AdjacencyMatrix[i][j] != 0; AdjacencyMatrix[i][j]--)
			AdjacencyMatrix[NumVertex - 1][NumVertex - 1]++;
		if (!contract)
			AdjacencyMatrix[NumVertex - 1][NumVertex - 1]++;
		for (int k = 0;k < NumVertex;k++) {
			if (AdjacencyMatrix[i][k] != 0) {
				AdjacencyMatrix[NumVertex - 1][k] += AdjacencyMatrix[i][k];
				if(!OrGraph)
					AdjacencyMatrix[k][NumVertex - 1] += AdjacencyMatrix[i][k];
			}
			if (AdjacencyMatrix[j][k] != 0) {
				AdjacencyMatrix[NumVertex - 1][k] += AdjacencyMatrix[j][k];
				if (!OrGraph)
					AdjacencyMatrix[k][NumVertex - 1] += AdjacencyMatrix[j][k];
			}
		}
		if(OrGraph)
			for (int k = 0;k < NumVertex;k++) {
				if (AdjacencyMatrix[k][i] != 0)
					AdjacencyMatrix[k][NumVertex - 1] += AdjacencyMatrix[k][i];
				if (AdjacencyMatrix[k][j] != 0)
					AdjacencyMatrix[k][NumVertex - 1] += AdjacencyMatrix[k][j];
			}
		delVertex(v1);
		delVertex(v2);
	}
	void duplicat_reproductVertex(unsigned int v, bool reproduct) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		unsigned int v_ = NameVertex[NumVertex - 1] + 1;
		addVertex(v_);
		for (int k = 0;k < NumVertex-1;k++)
			if (AdjacencyMatrix[i][k] != 0)
				AdjacencyMatrix[NumVertex - 1][k] += AdjacencyMatrix[i][k];
		for (int k = 0;k < NumVertex-1;k++)
			if (AdjacencyMatrix[k][i] != 0)
				AdjacencyMatrix[k][NumVertex - 1] += AdjacencyMatrix[k][i];
		if(AdjacencyMatrix[i][i]!=0)
			AdjacencyMatrix[NumVertex - 1][NumVertex - 1] += AdjacencyMatrix[i][i];
		if (reproduct) {
			AdjacencyMatrix[NumVertex - 1][i]++;
			AdjacencyMatrix[i][NumVertex - 1]++;
		}
		GraphView[0] = 0;
		GraphView[1] = 1;
		GraphView[2] = 0;
		TranslationToOtherRepresent();
	}
	void setGraph2(int AM[50][50], char bufConsole[100][101], unsigned int *v, unsigned int Names[100]) {
		memset(bufConsole, 0, 10100);
		int i = 0;
		cout << "Ведите граф в виде матрицы смежности:" << endl;
		do {
			cin.getline(bufConsole[i], 100);
			i++;
		} while ((i < 100) && (strcmp(bufConsole[i - 1], "end") != 0));
		*v = i - 1;
		for (i = 0;i < *v;i++) {
			int k = 0;
			for (int j = 0;j < *v+1;j++) {
				while (bufConsole[i][k] == ' ')
					k++;
				if (bufConsole[i][k] == '\0')
					break;
				if (j == 0)
					Names[i] = atoi(&bufConsole[i][k]);
				else
					AM[i][j - 1] = atoi(&bufConsole[i][k]);
				while (bufConsole[i][k] != ' '&&bufConsole[i][k] != '\0')
					k++;
			}
		}
	}
	void disjunctiveUnion_connectGraphs(int AM[50][50], unsigned int v, unsigned int Names[100], bool connect) {
		if (NumVertex > 50) {
			cout << "Исходный граф слишком большой!" << endl << endl;
			return;
		}
		unsigned int bufName[100] = { 0 };
		unsigned int bufNumV = NumVertex;
		for (int i = 0;i < v;i++) {
			int l = 0;
			for (;l < NumVertex && Names[i] != NameVertex[l];l++);
			if (l < NumVertex) {
				bufName[i] = l;
				if (AM[0][i] != 0)
					AdjacencyMatrix[bufName[0]][l] += AM[0][i];
			}
			else {
				if (connect)
					for (int k = 0;k < NumVertex;k++) {
						AdjacencyMatrix[NumVertex][k] = 1;
						AdjacencyMatrix[k][NumVertex] = 1;
					}
				bufName[i] = NumVertex;
				NameVertex[NumVertex] = Names[i];
				NumVertex++;
				AdjacencyMatrix[bufName[0]][NumVertex - 1] = AM[0][i];
			}
		}
		for (int i = 1;i < v;i++)
			for (int j = 0;j < v;j++)
			{
				if(bufName[j]<bufNumV&&bufName[i] < bufNumV)
					AdjacencyMatrix[bufName[i]][bufName[j]] += AM[i][j];
				else
					AdjacencyMatrix[bufName[i]][bufName[j]] = AM[i][j];
			}
		GraphView[0] = 0;
		GraphView[1] = 1;
		GraphView[2] = 0;
		TranslationToOtherRepresent();
	}
	void productGraphs(int AM[50][50], unsigned int v) {
		if (NumVertex*v > 100) {
			cout << "Граф слишком большой!" << endl << endl;
			return;
		}
		int bufAdjacencyMatrix[100][100] = { 0 };
		unsigned int bufNumVertex = NumVertex * v;
		unsigned int bufNameVertex[100] = { 0 };
		for (int i = 1;i < NumVertex+1;i++)
			for (int j = 1;j < v+1;j++)
				bufNameVertex[(i-1)*v + j-1] = i * 10 + j;
		for (int i = 0;i < bufNumVertex;i++)
			for (int j = 0;j < bufNumVertex;j++) {
				if (i == j) {
					bufAdjacencyMatrix[i][j] = AdjacencyMatrix[(bufNameVertex[i] / 10)-1][(bufNameVertex[i] / 10)-1] + AM[(bufNameVertex[i] % 10)-1][(bufNameVertex[i] % 10)-1];
					continue;
				}
				if ((bufNameVertex[i] / 10) == (bufNameVertex[j] / 10)) {
					bufAdjacencyMatrix[i][j] = AM[(bufNameVertex[i] % 10)-1][(bufNameVertex[j] % 10)-1];
					continue;
				}
				if ((bufNameVertex[i] % 10) == (bufNameVertex[j] % 10)) {
					bufAdjacencyMatrix[i][j] = AdjacencyMatrix[(bufNameVertex[i] / 10)-1][(bufNameVertex[j] / 10)-1];
					continue;
				}
				else
					bufAdjacencyMatrix[i][j] = 0;
			}
		NumVertex = bufNumVertex;
		memcpy(NameVertex, bufNameVertex, 100);
		memcpy(AdjacencyMatrix, bufAdjacencyMatrix, 10000);
		GraphView[0] = 0;
		GraphView[1] = 1;
		GraphView[2] = 0;
		TranslationToOtherRepresent();
	}
	void findSceletBFS(unsigned int v) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int AM[100][100] = { 0 };
		bool pass[100] = { 0 };
		queue<unsigned int> QueVertex;
		QueVertex.push(i);
		pass[i] = true;
		while (!QueVertex.empty()) {
			unsigned int vert = QueVertex.front();
			QueVertex.pop();
			for (int j = 0;j < NumVertex;j++)
				if (AdjacencyMatrix[vert][j] != 0 && !pass[j]) {
					QueVertex.push(j);
					pass[j] = true;
					AM[vert][j]++;
					AM[j][vert]++;
				}
		}
		cout << "Остов графа с узлом-источником " << v << ":"<< endl;
		cout << "/--";
		for (int i = 0;i < NumVertex;i++)
			cout << NameVertex[i] << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i] << "  ";
			for (int j = 0;j < NumVertex;j++)
				cout << AM[i][j] << " ";
			cout << endl;
		}
		cout << endl;
	}
	void findSceletDFS(unsigned int v) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int AM[100][100] = { 0 };
		bool pass[100] = { 0 };
		stack<unsigned int> StackV;
		stack<unsigned int> StackDel;
		StackV.push(i);
		unsigned int vert = 0;
		while (!StackV.empty()) {
			vert = StackV.top();
			if (!StackDel.empty() && vert == StackDel.top()) {
				StackV.pop();
				StackDel.pop();
			}
			else if(pass[vert])
				StackV.pop();
			else {
				if (!StackDel.empty()) {
					AM[vert][StackDel.top()]++;
					AM[StackDel.top()][vert]++;
				}
				pass[vert] = true;
				StackDel.push(vert);
				for (int j = 0;j < NumVertex;j++)
					if (AdjacencyMatrix[vert][j] != 0 && !pass[j])
						StackV.push(j);
			}
		}
		cout << "Остов графа с узлом-источником " << v << ":" << endl;
		cout << "/--";
		for (int i = 0;i < NumVertex;i++)
			cout << NameVertex[i] << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i] << "  ";
			for (int j = 0;j < NumVertex;j++)
				cout << AM[i][j] << " ";
			cout << endl;
		}
		cout << endl;
	}
	void findMinSceletPrim(unsigned int v) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int AM[100][100] = { 0 };
		bool pass[100] = { 0 };
		unsigned int verti = 0, vertj = 0;
		int min = INT_MAX;
		pass[i] = true;
		for (int k = 0;k < NumVertex - 1;k++) {
			for (i = 0;i < NumVertex;i++)
				if (pass[i])
					for (int j = 0;j < NumVertex;j++)
						if (!pass[j] && AdjacencyMatrix[i][j] != 0 && AdjacencyMatrix[i][j] < min) {
							min = AdjacencyMatrix[i][j];
							verti = i;
							vertj = j;
						}
			pass[vertj] = true;
			AM[verti][vertj] = min;
			AM[vertj][verti] = min;
			min = INT_MAX;
		}
		cout << "Минимальный остов графа с узлом-источником " << v << ":" << endl;
		cout << "/--";
		for (int i = 0;i < NumVertex;i++)
			cout << NameVertex[i] << "-";
		cout << endl;
		cout << "|" << endl;
		for (int i = 0;i < NumVertex;i++) {
			cout << NameVertex[i] << "  ";
			for (int j = 0;j < NumVertex;j++)
				cout << AM[i][j] << " ";
			cout << endl;
		}
		cout << endl;
	}
	void Bellman_Ford(unsigned int v, unsigned int inf) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		int buf = i;
		int distance[100] = { 0 };
		for (int l = 0;l < 100;l++)
			distance[l] = inf;
		distance[i] = 0;
		for (i = 0;i < NumVertex;i++)
			for (int j = 0;j < NumVertex;j++)
				if (i != j && AdjacencyMatrix[i][j] == 0)
					AdjacencyMatrix[i][j] = inf;
		unsigned int path[100][100] = { 0 };
		for(int k=0;k<NumVertex;k++)
			for (i = 0;i < NumVertex;i++)
				for (int j = 0;j < NumVertex;j++)
					if (distance[i] != inf && AdjacencyMatrix[i][j] != inf && distance[i] + AdjacencyMatrix[i][j] < distance[j]) {
						distance[j] = distance[i] + AdjacencyMatrix[i][j];
						memset(path[j], 0, 100 * sizeof(int));
						int l = 0;
						for (;path[i][l] != 0;l++)
							path[j][l] = path[i][l];
						path[j][l] = NameVertex[i];
					}
		for (i = 0;i < NumVertex;i++)
			if (distance[i] == inf)
				cout << v << " -> " << NameVertex[i] << ": " << "INFINUM" << endl;
			else if (buf != i) {
				for (int l = 0;path[i][l] != 0;l++)
					cout << path[i][l] << " -> ";
				cout << NameVertex[i] << ": " << distance[i] << endl;
			}
		cout << endl;
	}
	void maxmin(unsigned int v, unsigned int inf) {
		int i = 0;
		for (;i < NumVertex;i++)
			if (NameVertex[i] == v)
				break;
		if (i == NumVertex) {
			cout << "Такой вершины не существует!" << endl << endl;
			return;
		}
		unsigned int path[100][100] = { 0 };
		bool pass[100] = { 0 };
		int weight[100] = { 0 };
		for (int l = 0;l < 100;l++)
			weight[l] = inf;
		pass[i] = 1;
		path[i][0] = NameVertex[i];
		unsigned int last = i;
		for (i = 0;i < NumVertex;i++)
			for (int j = 0;j < NumVertex;j++)
				if (i != j && AdjacencyMatrix[i][j] == 0)
					AdjacencyMatrix[i][j] = inf;
		for (int j = 0;j < NumVertex - 1;j++) {
			for (int l = 0;l < NumVertex;l++)
				if (!pass[l] && AdjacencyMatrix[last][l] != inf)
					if (weight[l] == inf || AdjacencyMatrix[last][l] > weight[l]) {
						if (AdjacencyMatrix[last][l] > weight[last])
							weight[l] = weight[last];
						else
							weight[l] = AdjacencyMatrix[last][l];
						memset(path[l], 0, 100 * sizeof(int));
						int k = 0;
						for (;path[last][k] != 0;k++)
							path[l][k] = path[last][k];
						path[l][k] = NameVertex[l];
					}
			int max = INT_MIN;
			for (int l = 0;l < NumVertex;l++)
				if (!pass[l] && weight[l] != inf && weight[l] > max)
					max = l;
			if (max == INT_MIN)
				break;
			pass[max] = 1;
			last = max;
		}
		for (i = 1;i < NumVertex;i++)
			if (weight[i] == inf)
				cout << v << " -> " << NameVertex[i] << ": " << "INFINUM" << endl;
			else {
				for (int l = 0;;l++) {
					cout << path[i][l];
					if (path[i][l + 1] != 0)
						cout << " -> ";
					else break;
				}
				cout << ": " << weight[i] << endl;
			}
		cout << endl;
	}
	void maxFlow(void) {
		if (getSourceSink() != 2) {
			cout << "Несколько истоков или стоков!" << endl << endl;
			return;
		}
		unsigned int flow[100][100] = { 0 };
		int i = SourceSink[0];
		while (1) {
			bool pass[100] = { 0 };
			queue<unsigned int> QueVertex;
			int parents[100] = { 0 };
			QueVertex.push(i);
			pass[i] = true;
			while (!QueVertex.empty()) {
				unsigned int vert = QueVertex.front();
				QueVertex.pop();
				int j = 0;
				for (;j < NumVertex;j++)
					if (!pass[j] && ((AdjacencyMatrix[vert][j] != 0 && AdjacencyMatrix[vert][j] != flow[vert][j]) || (AdjacencyMatrix[j][vert] != 0 && flow[j][vert] != 0))) {//???
						parents[j + 1] = vert + 1;
						if (AdjacencyMatrix[j][vert] != 0)
							parents[j + 1] *= -1;
						if (j == SourceSink[1])
							break;
						QueVertex.push(j);
						pass[j] = true;
					}
				if (j != NumVertex)
					while (!QueVertex.empty())
						QueVertex.pop();
			}//если нет пути - считаем поток и заканчиваем

			if (parents[SourceSink[1]+1] == 0)
			{
				unsigned int MaxFlow = 0;
				for (int j = 0;j < NumVertex;j++)
					MaxFlow += flow[SourceSink[0]][j];
				cout << "Максимальный поток: " << MaxFlow << endl << endl;
				return;
			}
			unsigned int nowVertex = SourceSink[1], min = INT_MAX;
			while (parents[nowVertex+1]-1 != SourceSink[0]) {
				if (parents[nowVertex+1] < 0) {
					if (flow[nowVertex][-1 * (parents[nowVertex + 1] + 1)] < min)
						min = flow[nowVertex][-1 * (parents[nowVertex + 1] + 1)];
				}
				else if ((AdjacencyMatrix[parents[nowVertex+1]-1][nowVertex] - flow[parents[nowVertex + 1] - 1][nowVertex])< min)
					min = AdjacencyMatrix[parents[nowVertex + 1] - 1][nowVertex] - flow[parents[nowVertex + 1] - 1][nowVertex];
				if (parents[nowVertex+1] < 0)
					nowVertex = -1 * (parents[nowVertex + 1] + 1);
				else
					nowVertex = parents[nowVertex + 1] - 1;
			}
			if ((AdjacencyMatrix[parents[nowVertex + 1] - 1][nowVertex] - flow[parents[nowVertex + 1] - 1][nowVertex]) < min)
				min = AdjacencyMatrix[parents[nowVertex + 1] - 1][nowVertex] - flow[parents[nowVertex + 1] - 1][nowVertex];
			nowVertex = SourceSink[1];
			while (parents[nowVertex+1]-1 != SourceSink[0]) {
				if (parents[nowVertex+1] < 0)
					flow[nowVertex][-1 * (parents[nowVertex + 1] + 1)] -= min;
				else
					flow[parents[nowVertex + 1] - 1][nowVertex] += min;
				if (parents[nowVertex + 1] < 0)
					nowVertex = -1 * (parents[nowVertex + 1] + 1);
				else
					nowVertex = parents[nowVertex + 1] - 1;
			}
			flow[parents[nowVertex + 1] - 1][nowVertex] += min;
			//идём по parants пока не дойдём от Source[0] до Source[1] (ищем min)
			//... (записываем min во flow)
		}
	}
};

int main()
{
	setlocale(LC_CTYPE, "rus");
	int Method;
	bool Type;
	char bufConsole[100][101] = { 0 };
	int Matrix[100][100] = { 0 }, max = 0;
	unsigned int v = 0, e = 0, av[100] = { 0 };
	while (1) {
		cout << "Каким способом хотите задать граф? (0-списком смежности; 1-матрицей смежности; 2-матрицей инцидентности; 3-выйти из программы): ";
		cin >> Method;
		if (Method == 3)
			return 0;
		cout << "Граф ориентированный? (0-нет; 1-да): ";
		cin >> Type;
		if (Type != 0 && Type != 1) {
			cout << "Вы ввели неверный тип!";
			return 1;
		}
		cout << "Введите ";
		switch (Method) {
		case 0:
			if (!Type)
				cout << "список смежности в виде: \n\n1:3\n2:\n3:1 3\nend\n\nГде 1 вершина соединена с 3, 2 - висячая, 3 - соединена с 1 и имеет петлю.\n\n";
			else
				cout << "список смежности в виде: \n\n1:2 4\n2:1\n3:\n4:4\nend\n\nГде из 1 вершины идёт дуга в вершины 2 и 4, из 2 - только в 1, 3 - весячая, 4 - имеет только петлю.\n\n";
			break;
		case 1:
			if (!Type)
				cout << "матрицу смежности в виде: \n\n1 2 0 1\n2 0 0 0\n0 0 0 0\n1 0 0 0\nend\n\nГде 1 вершина соединена двумя рёбрами с 2 и одной - с 4 и имеет петлю, 2 - с 1, 3 - висячая, 4 - соединена с 1.\n\n";
			else
				cout << "матрицу смежности в виде: \n\n1 2 0 0\n0 0 0 1\n0 0 0 0\n1 0 0 0\nend\n\nГде из 1 вершины идёт две дуги в 2, и 1 имеет петлю, из 2 - дуга в 4, 3 - висячая, из 4 - дуга в 1.\n\n";
			break;
		case 2:
			if (!Type)
				cout << "матрицу инцидентности в виде: \n\n1 1 1 0\n1 1 0 0\n0 0 0 0\n0 0 1 2\nend\n\nГде 1 вершина соединена двумя рёбрами с 2 и одной - с 4, 2 - с 1 дважды, 3 - висячая, 4 - соединена с 1 и имеет петлю.\n\n";
			else
				cout << "матрицу инцидентности в виде: \n\n-1  1  1 0\n 1 -1  0 0\n 0  0  0 0\n 0  0 -1 2\nend\n\nГде из 1 вершины идёт дуга в 2 и 4, из 2 - дуга в 1, 3 - висячая, 4 - имеет петлю.\n\n";
			break;
		default:
			cout << "Вы ввели неверный способ!";
			system("cls");
			continue;
		}
		int i = 0;
		cin.get();
		do {
			cin.getline(bufConsole[i], 100);
			i++;
		} while ((i < 100) && (strcmp(bufConsole[i - 1], "end") != 0));
		v = i - 1;
		Graph MainG;
		switch (Method) {
		case 0: {
			int j = 0;
			for (i = 0;i < v;i++) {
				int k = 0;
				for (j = 0;j < 100 && bufConsole[i][k] != '\0';j++) {
					while (bufConsole[i][k] == ' ' || bufConsole[i][k] == ':')
						k++;
					Matrix[i][j] = atoi(&bufConsole[i][k]);
					if (bufConsole[i][k] == '\0')
						break;
					if (k == 0)
						j--;
					while (bufConsole[i][k] != ' '&&bufConsole[i][k] != '\0'&&bufConsole[i][k] != ':')
						k++;
				}
				av[i] = j;
			}
			Graph G(Type, Matrix, v, av);
			MainG = G;
			break;
		}
		case 1: {
			for (i = 0;i < v;i++) {
				int k = 0;
				for (int j = 0;j < v;j++) {
					while (bufConsole[i][k] == ' ')
						k++;
					if (bufConsole[i][k] == '\0')
						break;
					Matrix[i][j] = atoi(&bufConsole[i][k]);
					if (max < Matrix[i][j])
						max = Matrix[i][j];
					while (bufConsole[i][k] != ' '&&bufConsole[i][k] != '\0')
						k++;
				}
			}
			Graph G(Type, Matrix, v);
			MainG = G;
			break;
		}
		case 2: {
			int j = 0;
			for (i = 0;i < v;i++) {
				int k = 0;
				for (j = 0;j < 100;j++) {
					while (bufConsole[i][k] == ' ')
						k++;
					if (bufConsole[i][k] == '\0')
						break;
					Matrix[i][j] = atoi(&bufConsole[i][k]);
					while (bufConsole[i][k] != ' '&&bufConsole[i][k] != '\0')
						k++;
				}
				e = j;
			}
			Graph G(Type, Matrix, v, e);
			MainG = G;
			break;
		}
		}
		cout << endl;
		char Command[100] = { 0 };
		int AM[50][50] = { 0 };
		unsigned int Names[100] = { 0 };
		while (1) {
			cout << "Введите команду: ";
			cin.getline(Command, 99);
			if (!strcmp(Command, "get AL"))
				MainG.getAdjacencyList();
			else if (!strcmp(Command, "get AM"))
				MainG.getAdjacencyMatrix();
			else if (!strcmp(Command, "get IM"))
				MainG.getIncidenceMatrix();
			else if (!strcmp(Command, "get info"))
				MainG.getInfo();
			else if (!strncmp(Command, "get power", 9))
				MainG.getPowerVertex((unsigned int)atoi(&Command[10]));
			else if (!strcmp(Command, "get info v"))
				if (!Type)
					MainG.getHengingIsolatedVert();
				else
					MainG.getSourceSink();
			else if (!strncmp(Command, "get dist", 8)) {
				unsigned int l, v1 = atoi(&Command[9]);
				for (l = 10;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'get dist'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					if (MainG.getDistance(v1, v2, OUTPUT))
						cout << "Путь не найден" << endl << endl;
				}
			}
			else if (!strncmp(Command, "get eccent", 10))
				MainG.getEccentricity(atoi(&Command[11]), OUTPUT);
			else if (!strcmp(Command, "get radius"))
				MainG.getRadius(OUTPUT);
			else if (!strcmp(Command, "get diameter"))
				MainG.getDiameter(OUTPUT);
			else if (!strcmp(Command, "get center"))
				MainG.getCenter();
			else if (!strcmp(Command, "get peripheral"))
				MainG.getPeripheralVertices();
			else if (!strncmp(Command, "add v", 5))
				MainG.addVertex((unsigned int)atoi(&Command[6]));
			else if (!strncmp(Command, "del v", 5))
				MainG.delVertex((unsigned int)atoi(&Command[6]));
			else if (!strncmp(Command, "add e", 5)) {
				unsigned int l, v1 = atoi(&Command[6]);
				for (l = 7;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'add e'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					MainG.addEdge(v1, v2);
				}
			}
			else if (!strncmp(Command, "del e", 5)) {
				unsigned int l, v1 = atoi(&Command[6]);
				for (l = 7;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'del e'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					MainG.delEdge(v1, v2);
				}
			}
			else if (!strcmp(Command, "get complement"))
				MainG.findComplementGraph();
			else if (!strncmp(Command, "subdiv", 6)) {
				unsigned int l, v1 = atoi(&Command[7]);
				for (l = 8;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'subdivid'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					MainG.subdividGraphEdge(v1, v2);
				}
			}
			else if (!strncmp(Command, "contract", 8)) {
				unsigned int l, v1 = atoi(&Command[9]);
				for (l = 10;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'contract'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					MainG.contractionEdge_identVertex(v1, v2, CONTRACTION);
				}
			}
			else if (!strncmp(Command, "ident", 5)) {
				unsigned int l, v1 = atoi(&Command[6]);
				for (l = 7;Command[l] != '\0'&&Command[l] != ' '&&l < 99;l++);
				if (l == 99 || Command[l] == '\0')
					cout << "Неверно введена команда 'ident'!" << endl << endl;
				else {
					unsigned int v2 = atoi(&Command[l + 1]);
					MainG.contractionEdge_identVertex(v1, v2, IDENTIFICATION);
				}
			}
			else if (!strncmp(Command, "duplicate", 9))
				MainG.duplicat_reproductVertex((unsigned int)atoi(&Command[10]), DUPLICATION);
			else if (!strncmp(Command, "reproduct", 9))
				MainG.duplicat_reproductVertex((unsigned int)atoi(&Command[10]), REPRODUCTION);
			else if (!strncmp(Command, "union", 5)) {
				MainG.setGraph2(AM, bufConsole, &v, Names);
				MainG.disjunctiveUnion_connectGraphs(AM, v, Names, UNION);
			}
			else if (!strncmp(Command, "connect", 8)) {
				MainG.setGraph2(AM, bufConsole, &v, Names);
				MainG.disjunctiveUnion_connectGraphs(AM, v, Names, CONNECT);
			}
			else if (!strncmp(Command, "product", 8)) {
				MainG.setGraph2(AM, bufConsole, &v, Names);
				MainG.productGraphs(AM, v);
			}
			else if (!strncmp(Command, "scelet BFS", 10))
				MainG.findSceletBFS((unsigned int)atoi(&Command[11]));
			else if (!strncmp(Command, "scelet DFS", 10))
				MainG.findSceletDFS((unsigned int)atoi(&Command[11]));
			else if (!strncmp(Command, "scelet Prim", 11))
				MainG.findMinSceletPrim((unsigned int)atoi(&Command[12]));
			else if (!strncmp(Command, "Ford", 4))
			{
				unsigned int inf = max * v;
				MainG.Bellman_Ford((unsigned int)atoi(&Command[5]), inf);
			}
			else if (!strncmp(Command, "maxmin", 6))
			{
				unsigned int inf = max * v;
				MainG.maxmin((unsigned int)atoi(&Command[7]), inf);
			}
			else if (!strncmp(Command, "flow", 4))
				MainG.maxFlow();
			else if (!strcmp(Command, "end")) {
				memset(Command, 0, 100);
				break;
			}
			else if (!strcmp(Command, "help")) {
				cout << "Commands:\nget AL\nget AM\nget IM\nget info\nget power _\nget info v\nget dist _ _\nget eccent _\nget radius\nget diameter\nget center\nget peripheral\nadd v _\ndel v _\nadd e _ _\ndel e _ _\nget complement\nsubdiv _ _\ncontract _ _\nident _ _\nduplicate _\nreproduct _\nunion\nconnect\nproduct\nscelet BFS _\nscelet DFS _\nscelet Prim _\nFord _\nmaxmin _\nflow" << endl << endl;
			}
			else {
				cout << "Неверно введена команда!\nДля получения информации о командах введите 'help'" << endl << endl;
			}
			memset(Command, 0, 100);
		}
	}
	return 0;
}