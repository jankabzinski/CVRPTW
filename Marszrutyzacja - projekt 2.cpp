#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <algorithm>
#include <vector>
#include <list>
#include <ctime>
#include <set>
#include <iomanip>

using namespace std;
int n_C;// number of clients
int truckCap;
fstream fileIn, fileOut;
vector <int> templatka, candid, backup;

double** distT;//tablica odleglosci

struct Garage
{
	int dueTime, x, y;
};

Garage Wrhse;

struct Truck
{
	int cap = truckCap;
	vector <int> nClients;
	list <double> czasy;
	double time = 0.0;
};

struct Town
{
	double x, y, r, alpha;
	int id, need, readyT, dueT, serviceT;
};
Town Client[2302];

struct BestFit
{
	Truck tab[2302];
	int nTrucks;
};

bool wczytanieDanych()
{
	string garbage;
	int g, id;
	for (int i = 0; i < 5; i++)
		fileIn >> garbage;

	fileIn >> truckCap;

	for (int p = 0; p < 12; p++)
		fileIn >> garbage;

	fileIn >> g >> Wrhse.x >> Wrhse.y >> g >> g >> Wrhse.dueTime >> g;

	Client[0].id = 0;
	Client[0].x = Wrhse.x;
	Client[0].y = Wrhse.y;

	while (fileIn >> id)
	{
		Client[id].id = id;
		fileIn >> Client[id].x >> Client[id].y >> Client[id].need;
		fileIn >> Client[id].readyT >> Client[id].dueT >> Client[id].serviceT;
		Client[id].x -= Wrhse.x;
		Client[id].y -= Wrhse.y;
		Client[id].r = pow(pow(Client[id].x, 2) + pow(Client[id].y, 2), 0.5);
		Client[id].alpha = atan(Client[id].y / Client[id].x) * 180 / M_PI;

		if (Wrhse.dueTime < max(double(Client[id].readyT), Client[id].r) + Client[id].serviceT + Client[id].r || Client[id].r >= Client[id].dueT)
			return true;
	}
	n_C = id;
	return false;
}

void ogarnij(BestFit& t, int truck)
{
	set <int> cur;
	vector <int> klienci, best;
	best = t.tab[truck].nClients;
	bool istnienia;
	int horyzont, cap;
	int proby = 0, index, ofset;
	double lastTime, shortest = *(--(--t.tab[truck].czasy.end())) + Client[*(--(--t.tab[truck].nClients.end()))].r;
	while (proby < 15)
	{
		cur.clear();
		klienci.clear();
		klienci.push_back(0);
		for (auto i = ++best.begin(); i != --best.end(); i++)
		{
			index = 0;
			while (index < backup.size() && backup[index] != *i)
				index++;

			horyzont = min(25, min(index, int(backup.size()) - 1 - index));
			if (horyzont > 0)
			{
				for (int r = 0; r < 30; r++)
				{
					ofset = rand() % (2 * horyzont) - horyzont;
					if (cur.find(backup[index + ofset]) == cur.end())
					{
						klienci.push_back(backup[index + ofset]);
						cur.insert(backup[index + ofset]);
						break;
					}
				}
			}
			else
			{
				if (index == 0)
				{
					horyzont = min(25, int(candid.size()));
					for (int r = 0; r < 30; r++)
					{
						ofset = rand() % horyzont;
						if (cur.find(backup[index + ofset]) == cur.end())
						{
							klienci.push_back(backup[index + ofset]);
							cur.insert(backup[index + ofset]);
							break;
						}
					}
				}
				else
				{
					horyzont = max(-25, (int(candid.size()))*(-1) );
					for (int r = 0; r < 30; r++)
					{
						ofset = rand() % (-1*horyzont) + horyzont;
						if (cur.find(backup[index + ofset]) == cur.end())
						{
							klienci.push_back(backup[index + ofset]);
							cur.insert(backup[index + ofset]);
							break;
						}
					}
				}
			}
		}
		//sprawdzenie
		istnienia = true;
		if (klienci.size() > 1 && klienci.size() == best.size() - 1)
		{
			istnienia = false;
			auto h = ++klienci.begin();
			cap = truckCap - Client[*h].need;
			lastTime = max(Client[*h].r, double(Client[*h].readyT)) + Client[*h].serviceT;

			for (auto i = ++(++klienci.begin()); i != klienci.end(); i++)
			{
				if (lastTime + distT[*h][*i] < Client[*i].dueT)
				{
					if (cap - Client[*i].need < 0)
					{
						istnienia = true;
						break;
					}
					else
					{
						lastTime = max(lastTime + distT[*h][*i], double(Client[*i].readyT)) + Client[*i].serviceT;
						cap -= Client[*i].need;
					}
				}
				else
				{
					istnienia = true;
					break;
				}
				h++;
			}

			if (!istnienia)
			{
				if (//cap <= t.tab[truck].cap &&
					lastTime + Client[*(--klienci.end())].r < shortest )
				{
					shortest = lastTime + Client[*(--klienci.end())].r;
					best.push_back(0);
					best = klienci;
				}
				else
					proby++;
			}
			else
				proby++;
		}
		else
			proby++;
	}
	t.tab[truck].nClients = best;
	for (auto i = ++best.begin(); i != (--best.end()); i++)
	{
		auto x = backup.begin();
		for (; x != backup.end() && *x != *i; x++)
			;

		backup.erase(x);
	}

	t.tab[truck].time = shortest;
	t.tab[truck].czasy.clear();
	auto a = t.tab[truck].nClients.begin();
	auto b = ++t.tab[truck].nClients.begin();
	t.tab[truck].cap = truckCap;
	t.tab[truck].czasy.push_back(0.0);
	for (; b != --t.tab[truck].nClients.end(); b++)
	{
		t.tab[truck].cap -= Client[*b].need;
		double k = *(--t.tab[truck].czasy.end());
		t.tab[truck].czasy.push_back(max(k + distT[*a][*b], double(Client[*b].readyT)));
		t.tab[truck].czasy.push_back(max(k + distT[*a][*b], double(Client[*b].readyT)) + Client[*b].serviceT);
		++a;
	}
	t.tab[truck].czasy.push_back(Wrhse.dueTime);
}

void truckowanie(BestFit tbf[3])
{
	tbf[0] = tbf[2];//zerowanie 
	candid = templatka;
	backup = templatka;

	bool weltschmerz;
	bool istnienia; //mały żart ode mnie
	int curNTrucks = 1, x;
	auto curCli = candid.begin();

	tbf[0].tab[1].czasy.push_back(0);
	tbf[0].tab[1].nClients.push_back(0);
	tbf[0].tab[curNTrucks].czasy.push_back(Wrhse.dueTime);
	tbf[0].tab[curNTrucks].nClients.push_back(0);

	weltschmerz = false;
xd:
	while (!candid.empty())
	{
		curCli = candid.begin();
		for (int w = 0; w < candid.size(); w++)
		{
			if (tbf[0].tab[curNTrucks].cap - Client[*curCli].need >= 0)
			{
				/* a - iter po liscie czasu
				b - a-1
				c - iterator po vectorze
				d - indeks w vectorze
				e - d-1
				*/
				istnienia = false;
				auto d = tbf[0].tab[curNTrucks].nClients.begin();
				auto b = tbf[0].tab[curNTrucks].czasy.begin();
				auto a = ++tbf[0].tab[curNTrucks].czasy.begin();
				auto e = d;

				while (true)
				{
					e = d;
					++d;
					if (*a - *b >= distT[*d][*curCli] + distT[*e][*curCli] + Client[*curCli].serviceT)
					{
						if (*b + distT[*e][*curCli] < Client[*curCli].dueT &&
							*a >= max(distT[*e][*curCli] + *b, double(Client[*curCli].readyT)) + Client[*curCli].serviceT + distT[*curCli][*d])
						{
							istnienia = true;

							tbf[0].tab[curNTrucks].czasy.insert(a, max(distT[*e][*curCli] + *b, double(Client[*curCli].readyT)));
							tbf[0].tab[curNTrucks].czasy.insert(a, max(distT[*e][*curCli] + *b, double(Client[*curCli].readyT)) + Client[*curCli].serviceT);

							tbf[0].tab[curNTrucks].nClients.insert(d, *curCli);
							tbf[0].tab[curNTrucks].cap -= Client[*curCli].need;

							candid.erase(curCli);
							goto xd;
						}
					}
					if (a != --tbf[0].tab[curNTrucks].czasy.end())
					{
						++(++b); ++(++a);
					}

					else
						break;
				}
			}
			curCli++;
		}

		tbf[0].tab[curNTrucks].time = *(--(--tbf[0].tab[curNTrucks].czasy.end())) + Client[*(--(--tbf[0].tab[curNTrucks].nClients.end()))].r;
		
		if (!candid.empty())
		{
			if (candid.size() > 10 && weltschmerz == false)
			{
				ogarnij(tbf[0], curNTrucks);
				candid = backup;
				weltschmerz = true;
				goto xd;
			}
			else
			{
				weltschmerz = false;
				backup = candid;
			}
			curNTrucks++;
			tbf[0].tab[curNTrucks].czasy.push_back(0);
			tbf[0].tab[curNTrucks].nClients.push_back(0);
			tbf[0].tab[curNTrucks].czasy.push_back(Wrhse.dueTime);
			tbf[0].tab[curNTrucks].nClients.push_back(0);
		}
	}

	tbf[0].tab[curNTrucks].time = *(--(--tbf[0].tab[curNTrucks].czasy.end())) + Client[*(--(--tbf[0].tab[curNTrucks].nClients.end()))].r;

	if (curNTrucks < tbf[1].nTrucks)
	{
		tbf[1] = tbf[0];
		tbf[1].nTrucks = curNTrucks;
	}
}

void odleglosci()
{
	for (int i = 1; i <= n_C; i++)
	{
		for (int j = 1; j <= n_C; j++)
			distT[i][j] = pow(pow(Client[i].x - Client[j].x, 2) + pow(Client[i].y - Client[j].y, 2), 0.5);

	}
	for (int g = 1; g <= n_C; g++)
	{
		distT[0][g] = Client[g].r;
		distT[g][0] = Client[g].r;
	}
}

int main(int argc, char* argv[])
{
	double sum_of_time;
	srand(time(NULL));
	fileIn.open(argv[1], ios::in);
	fileOut.open(argv[2], ios::out);
	//fileIn.open("x.txt", ios::in);
	//fileOut.open("out.txt", ios::out);

	if (wczytanieDanych() == true)
	{
		cout << "-1" << endl;
		fileOut << "-1" << endl;
		fileIn.close();
	}
	else
	{
		fileIn.close();

		distT = new double* [n_C + 1]; //tworzenie tablicy odleglosci
		for (int i = 0; i <= n_C; i++)
			distT[i] = new double[n_C + 1];

		odleglosci(); // obliczanie odleglosci pomiedzy klientami

		BestFit tbf[3];
		tbf[1].nTrucks = 12234;
		int r[2200];
		for (int f = 1; f <= n_C; f++)
			r[f] = f;

		sort(r + 1, r + n_C + 1, [](int a, int b) {return (Client[a].alpha > Client[b].alpha); });
		for (int f = 1; f <= n_C; f++)
			templatka.push_back(r[f]);

		truckowanie(tbf);
		
		/*templatka.clear();
		sort(r + 1, r + n_C + 1, [](int a, int b) {return (Client[a].need > Client[b].need); });
		for (int f = 1; f <= n_C; f++)
			templatka.push_back(r[f]);

		truckowanie(tbf);*/
		
		templatka.clear();
		sort(r + 1, r + n_C + 1, [](int a, int b) {return (Client[a].alpha < Client[b].alpha); });
		for (int f = 1; f <= n_C; f++)
			templatka.push_back(r[f]);

		truckowanie(tbf);
	

		fileOut << tbf[1].nTrucks << " ";

		sum_of_time = 0.0;
		for (int v = 1; v <= tbf[1].nTrucks; v++)
		{
			sum_of_time += tbf[1].tab[v].time;
			//cout << v << ". " << tbf[1].tab[v].cap << " " << tbf[1].tab[v].nClients.size() - 2 << endl;
		}
		//cout << tbf[1].nTrucks << endl;
		//cout << setprecision(log10(sum_of_time) + 13) << sum_of_time << endl;

		fileOut << setprecision(log10(sum_of_time) + 13) << sum_of_time << endl;

		for (int v = 1; v <= tbf[1].nTrucks; v++)
		{
			for (auto iter = ++tbf[1].tab[v].nClients.begin(); iter != --tbf[1].tab[v].nClients.end(); iter++)
				fileOut << *iter << " ";

			fileOut << endl;
		}
		delete[] distT;
	}
	fileOut.close();
	return 0;
}