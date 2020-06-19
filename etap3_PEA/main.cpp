#include<iostream>
#include<fstream>
#include<string>
#include<conio.h>
#include<algorithm>
#include<ctime>
#include<iomanip>
#include<windows.h>
#include<list>
#include<vector>

using namespace std;

typedef int point2[];

int cities;
int** matrix;
int optimum;
string name;

//zmienne i funkcje wykorzystywane przy pomiarze czasu dzia³ania algorytmów
double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}


//dalsza czesc programu

void readFromFile(string fileName) {
	ifstream file;

	string dir = "instancje\\" + fileName;
	const char* fName = dir.c_str();
	file.open(fName);
	if (!file) {
		cout << "Nazwa pliku jest bledna!\n";
		return;
	}

	file >> name;
	file >> cities;

	matrix = new int*[cities];

	for (int i = 0; i < cities; i++) {
		matrix[i] = new int[cities];
		for (int j = 0; j < cities; j++)
			file >> matrix[i][j];
	}
	file >> optimum;
	file.close();
}

void displayMatrix() {
	cout << endl << "   ";
	for (int i = 0; i < cities; i++) {
		cout << setw(3) << "[" << i << "]";

	}
	cout << endl;
	for (int i = 0; i < cities; i++) {
		cout << "[" << i << "] ";
		for (int j = 0; j < cities; j++) {
			cout << setw(4) << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

vector<int> getOrder() {
	vector<int> order;
	int counter = cities;
	int pos;
	vector<int> buf;
	for (int i = 0; i < cities; i++)
		buf.push_back(i);

	while (counter > 0) {
		pos = rand() % counter;
		order.push_back(buf[pos]);
		buf.erase(buf.begin() + pos);
		counter--;
	}

	return order;
}

double getRandDouble() {
	double x;
	x = rand() % 100001;
	return x / 100000.0;
}

class Population {
	int size;
	double mutationRate;

	vector<int>* population;
	vector<int>* newPopulation;

	double* fitnessTab;
	double* summedFitnessTab;
	int* costTab;

	vector<int> BestEver;
	int BestCost;

	int getPathCost(vector<int> order);

	vector<int> chooseParent();
	vector<int> mutate(vector<int> parent);
	vector<int> crossover(vector<int> parent1, vector<int> parent2);
	
public:
	Population(int newSize, double newMutationRate);

	void calculateFitness();
	void normalizeFitness();

	void nextGeneration();

	vector<int> returnBestOrder();
	int returnBestCost();

	void printPopulation();
	void printFitness();
	void printBestOrder();
};

Population::Population(int newSize, double newMutationRate) {
	size = newSize;
	mutationRate = newMutationRate;

	population = new vector<int>[size];
	newPopulation = new vector<int>[size];

	for (int i = 0; i < size; i++)
		population[i] = getOrder();

	BestCost = 2147483647;

	fitnessTab = new double[size];
	summedFitnessTab = new double[size];
	costTab = new int[size];
}

int Population::getPathCost(vector<int> order) {
	int cost = 0;
	for (int i = 0; i < cities - 1; i++)
		cost += matrix[order[i]][order[i + 1]];
	cost += matrix[order.back()][order.front()];

	return cost;
}

void Population::calculateFitness() {
	int cost;
	vector<int> order;
	for (int j = 0; j < size; j++) {
		order = population[j];
		cost = getPathCost(order);

		costTab[j] = cost;

		fitnessTab[j] = 1.0 / (cost);
		
		if (cost < BestCost) {
			BestCost = cost;
			BestEver = order;
			cout << "\nNowy najlepszy wynik: " << BestCost << " roznica wynosi: " << (1.0*BestCost-optimum)/optimum*100.0 << "%";
		}
	}
	normalizeFitness();
}

void Population::normalizeFitness() {
	double sum = 0;
	
	for (int i = 0; i < size; i++) {
		fitnessTab[i] = pow(fitnessTab[i], 50);
		sum += fitnessTab[i];
	}

	for (int i = 0; i < size; i++) {
		fitnessTab[i] /= sum;
		if (i == 0)
			summedFitnessTab[i] = fitnessTab[i];
		else
			summedFitnessTab[i] = summedFitnessTab[i-1] + fitnessTab[i] ;
	}
}

vector<int> Population::chooseParent() {
	double random = getRandDouble();
	for (int i = 0; i < size; i++) {
		if (random < summedFitnessTab[i])
			return population[i];
	}

	return population[0];
}

vector<int> Population::mutate(vector<int> parent) {	
	int random1 = rand() % cities;
	int random2;
	
	do {
		random2 = rand() % cities;
	} while (random1 == random2);

	swap(parent[random1], parent[random2]);

	return parent;
}

vector<int> Population::crossover(vector<int> parent1, vector<int>parent2) {
	int start = rand() % (cities - 1);
	int end = rand() % (cities - start - 1);
	end += start + 1;

	vector<int>::const_iterator first1 = parent1.begin() + start;
	vector<int>::const_iterator last1 = parent1.begin() + end;

	vector<int> child(first1, last1);

	for (int i = 0; i < cities; i++) {

		for (size_t j = 0; j < child.size(); j++) {
			if (child[j] == parent2[i])
				break;
			if (j == child.size() - 1)
				child.push_back(parent2[i]);
		}
	}
	return child;
}


void Population::nextGeneration() {
	vector<int> parent1, parent2, child;
	for (int i = 0; i < size; i ++) {
		parent1 = chooseParent();
		parent2 = chooseParent();

		newPopulation[i] = crossover(parent1, parent2);
		double random = getRandDouble();
		if(random < mutationRate)
			newPopulation[i] = mutate(newPopulation[i]);
	}

	for (int i = 0; i < size; i++)
		population[i] = newPopulation[i];	
}

vector<int> Population::returnBestOrder() {
	return BestEver;
}

int Population::returnBestCost() {
	return BestCost;
}

void Population::printPopulation() {
	cout << endl;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < cities; j++)
			cout << population[i][j] << " ";
		cout << endl;
	}
}

void Population::printFitness() {
	cout << endl;
	for (int i = 0; i < size; i++)
		cout <<"szansa na wylosowanie: "<< fitnessTab[i]*100 <<"%   summedFitness:" << summedFitnessTab[i] 
			 << "	  dlugosc drogi: " << costTab[i] << endl;
}

void Population::printBestOrder() {
	cout << "\nNajlepsza droga: ";
	for (int i = 0; i < cities; i++)
		cout << BestEver[i] << "=>";
	cout << BestEver[0] << endl;
}

int GeneticAlgorithm() {
	int size = 800;
	double mutationRate = 0.05;

	int bestCost;
	vector<int> bestOrder;

	double time;
	int counter = 0;

	Population* population = new Population(size, mutationRate);
	population->calculateFitness();
	//bestCost = population->returnBestCost();
	//cout << "\nczas: " << counter * 1000 << "ms wynik: " << (double)(bestCost - optimum) / optimum * 100.0 << "%" << endl;
	StartCounter();
	for (int i = 0; i < 5000; i++) {
		population->nextGeneration();
		population->calculateFitness();

		bestCost = population->returnBestCost();
		bestOrder = population->returnBestOrder();
		//time = GetCounter();


		if ((double)(bestCost - optimum) / optimum == 0)
			break;
		/*if (time > 10000) {
			StartCounter();
			counter++;
			cout << "\nczas: " << counter * 10000 << "ms wynik: " << (double)(bestCost - optimum) / optimum * 100.0 << "%" << endl;
		}

		if (counter >= 60)
			break;

		if (time >= 60000) { // 1200s = 20min
			cout << "\nczas sie skonczyl! t = " << time << "ms";
			break;
		}*/
	}
	
	return bestCost;
}


int main() {
	srand(time(NULL));
	char  opt;
	string fileName;
	int sum;
	double time;
	double difference;

	do {
		cout << "\n==MENU==";
		cout << "\n1. Wczytaj z pliku";
		cout << "\n2. Wyswietl macierz";
		cout << "\n3. Algorytm genetyczny";
		cout << "\n0. Wyjdz\n";
		opt = _getche();
		switch (opt) {
		case '1': //wczytywanie z pliku
			cout << "\nPodaj nazwe pliku: ";
			cin >> fileName;
			readFromFile(fileName);
			break;

		case '2': //wyœwietlanie macierzy
			displayMatrix();
			break;
		case '3': // Algorytm Genetyczny
			//StartCounter();
			sum = GeneticAlgorithm();
			time = GetCounter();
			cout << "\nUkonczono Algorytm Genetyczny, wynik wynosi: " << sum;
			cout << "\nOptimum ma wartosc: " << optimum;
			difference = (double)(sum - optimum) / optimum;
			cout << "\nRoznica wynosi: " << difference * 100 << "%" << " czas: " << time;
			break;
		}
	} while (opt != '0');
	return 0;
}
