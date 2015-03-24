#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <stdlib.h> 
#include <valarray>
#include <vector>
#include <iomanip>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <queue>  
#include <stack> 

#define NORTH 1
#define EAST 2
#define SOUTH -1
#define WEST -2
#define UP 3
#define DOWN -3

using namespace std;

class wavefront_t {  
private:  
    int pos_x, pos_y, pos_layer, pathcost;  
public:  
	wavefront_t(int x, int y, int l, int pc) {
		pos_x = x;
		pos_y = y;
		pos_layer = l;
		pathcost = pc;
	} 
    int X() const { return pos_x; }  
    int Y() const { return pos_y; }
    int Layer() const { return pos_layer; }
    int PathCost() const { return pathcost; }         
};  
  
struct CompareNode : public std::binary_function<wavefront_t, wavefront_t, bool> {  
  bool operator()(const wavefront_t &lhs, const wavefront_t &rhs) const {  
     return lhs.PathCost() > rhs.PathCost();  
  }  
}; 

class gridInfo_t {  
private:  
    char val;  
public:  
	void clear () {
		val = 0;
	}
	void setReached (int r) {
		val = val + r;
	}
	void setPred (int p) {
		val = val + (p << 1);
	}
    unsigned short int getReached() const { return unsigned(val << 31) >> 31; }  
    short int getPred() const { return (val << 28) >> 29; }  
};  

void initGridInfo (std::vector<std::vector<std::vector<gridInfo_t> > > &gridInfo) {
	int n1 = gridInfo.size();
	int n2 = gridInfo[0].size();
	int n3 = gridInfo[0][0].size();
	for (int i=0; i<n1; i++) {
		for (int j=0; j<n2; j++) {
			for (int k=0; k<n3; k++) {
				gridInfo[i][j][k].clear();
			}
		}
	}
}

void readGrid (char* name, int *sx, int *sy, int *bendPenalty, int *viaPenalty, std::vector<std::vector<std::vector<short int> > > &gridCost) {
	std::string str;
	std::ifstream InFile (name);
	
	int sxv, syv,bp,vp;
	if (InFile.is_open()) {
		getline (InFile,str);
		istringstream buff(str);
		buff >> sxv >> syv >> bp >> vp;
		gridCost.resize(2, std::vector< std::vector<short int> >(syv, std::vector<short int>(sxv,0)));
		for (int i=0; i<2; i++) {
			for (int j=0; j<syv; j++) {
				getline (InFile,str);
				istringstream buff(str);
				for (int k=0; k<sxv; k++) {
					short int tmp;
					buff >> tmp;
					gridCost[i][j][k] = tmp;
				}
			}
		}
		*sx = sxv;
		*sy = syv;
		*bendPenalty = bp;
		*viaPenalty = vp;
	}
	//Close file
	InFile.close();
}

void readNetlist (char* name, int *nNetlist, std::vector<std::vector<int> > &netlist) {
	std::string str;
	std::ifstream InFile (name);
	
	int nl;
	if (InFile.is_open()) {
		getline (InFile,str);
		istringstream buff(str);
		buff >> nl;
		netlist.resize( nl , std::vector<int>( 6 , 0 ) );
		for (int i=0; i<nl; i++) {
			getline (InFile,str);
			istringstream buff(str);
			int tmp;
			buff >> tmp;
			for (int j=0; j<6; j++) {
				buff >> tmp;
				netlist[i][j] = tmp;
			}
			netlist[i][0] = netlist[i][0] - 1;
			netlist[i][3] = netlist[i][3] - 1;
		}
		*nNetlist = nl;
	}
	//Close file
	InFile.close();
}

struct coord {
  int x;
  int y;
  int layer;
} ;

void pointingCoord (short int p, coord &out) {
	switch(p){
		case NORTH  :
		   out.y += 1;
		   break; 
		case EAST  :
		   out.x += 1;
		   break; 
		case SOUTH  :
		   out.y -= 1;
		   break; 
		case WEST  :
		   out.x -= 1;
		   break; 
		case UP  :
		   out.layer += 1;
		   break; 
		case DOWN  :
		   out.layer -= 1;
		   break; 
		default : 
		   break;
	}
}

void expand (coord S, coord T, coord M, int bendPenalty, int viaPenalty, short int dir, std::vector<std::vector<std::vector<short int> > > &gridCost, std::vector<std::vector<std::vector<gridInfo_t> > > &gridInfo, wavefront_t &C, priority_queue<wavefront_t,vector<wavefront_t>, CompareNode > &wavefront) {
	int mx = M.x; int my = M.y; int ml = M.layer;
	int sx = S.x; int sy = S.y; int sl = S.layer;
	int ny = gridCost[0].size();
	int nx = gridCost[0][0].size();
	
	//cout << "   DEBUG " << wavefront.size() << endl;
	if ( (ml < 2 && ml >= 0) && (my < ny && my >= 0) && (mx < nx && mx >= 0) ) {
		if ((gridInfo[ml][my][mx].getReached() == 0) && (gridCost[ml][my][mx] >= 0) ) {
			gridInfo[ml][my][mx].setReached(1);
			gridInfo[ml][my][mx].setPred(dir);
			int costacc = C.PathCost() + gridCost[ml][my][mx];
			if (((C.X() != sx)||(C.Y() != sy)||(C.Layer() != sl))&&(gridInfo[ml][my][mx].getPred() != gridInfo[C.Layer()][C.Y()][C.X()].getPred())) {
				costacc += bendPenalty;
			}
			if (C.Layer() != ml) { costacc += viaPenalty - bendPenalty; }
			//costacc += abs(T.x - mx) + abs(T.y - my) - abs(T.x - C.X()) - abs(T.y - C.Y());
			wavefront.push( wavefront_t(mx, my, ml, costacc ) );
			//cout << "   " << mx << " " << my << " " << costacc << " " << dir << endl;
		}
	}
}

void mazeRouter (coord S, coord T, int nx, int ny, int bendPenalty, int viaPenalty, std::vector<std::vector<std::vector<short int> > > &gridCost, std::vector<std::vector<std::vector<gridInfo_t> > > &gridInfo, bool &success, std::stack<short int> &succ) {
	int sx = S.x;
	int sy = S.y;
	int sl = S.layer;
	int tx = T.x;
	int ty = T.y;
	int tl = T.layer;
	int costacc;
	priority_queue<wavefront_t,vector<wavefront_t>, CompareNode > wavefront;
	initGridInfo (gridInfo);
	gridInfo[sl][sy][sx].setReached(1);
	costacc = gridCost[sl][sy][sx];
	//costacc += abs(tx - sx) + abs(ty - sy);
	wavefront.push( wavefront_t(sx, sy, sl, costacc) ); 
	success = false;
	
	while (1) {
		
		if ( wavefront.empty() ) {
			success = false;
			break;
		}
		
		wavefront_t C = wavefront.top();
		
		//cout << "** " << C.Layer() << " " << C.X() << " " << C.Y() << " " << C.PathCost() << endl;
		
		if ( (C.X() == tx) && (C.Y() == ty) ) {
			if (C.Layer() != tl) {
				short int dir;
				if (tl==0) { dir = UP; } else { dir = DOWN; }
				gridInfo[tl][ty][tx].setReached(1);
				gridInfo[tl][ty][tx].setPred(dir);
				int costacc = C.PathCost() + gridCost[tl][ty][tx];
				costacc += viaPenalty;
				wavefront.push( wavefront_t(tx, ty, tl, costacc ) );
			} else {
				coord c = T;
				//cout << c.x << " " << sx << " " << c.y << " " << sy << " " << c.layer << " " << sl << endl;
				while ((c.x != sx)||(c.y != sy)||(c.layer != sl)) {
					gridCost[c.layer][c.y][c.x] = -1;
					short int tmp = gridInfo[c.layer][c.y][c.x].getPred();
					pointingCoord (tmp, c);
					succ.push(tmp);
				}
				gridCost[c.layer][c.y][c.x] = -1;
				success = true;
				break;
			}
		}
		
		coord M;
		
		M.x = C.X(); M.y = C.Y() + 1; M.layer = C.Layer();
		expand (S, T, M, bendPenalty, viaPenalty, SOUTH, gridCost, gridInfo, C, wavefront);
		
		M.x = C.X(); M.y = C.Y() - 1; M.layer = C.Layer();
		expand (S, T, M, bendPenalty, viaPenalty, NORTH, gridCost, gridInfo, C, wavefront);
		
		M.x = C.X() + 1; M.y = C.Y(); M.layer = C.Layer();
		expand (S, T, M, bendPenalty, viaPenalty, WEST, gridCost, gridInfo, C, wavefront);
		
		M.x = C.X() - 1; M.y = C.Y(); M.layer = C.Layer();
		expand (S, T, M, bendPenalty, viaPenalty, EAST, gridCost, gridInfo, C, wavefront);
		
		M.x = C.X(); M.y = C.Y(); M.layer = C.Layer() + 1;
		expand (S, T, M, bendPenalty, viaPenalty, DOWN, gridCost, gridInfo, C, wavefront);
		
		M.x = C.X(); M.y = C.Y(); M.layer = C.Layer() - 1;
		expand (S, T, M, bendPenalty, viaPenalty, UP, gridCost, gridInfo, C, wavefront);
		
		wavefront.pop(); 
	}
}

void traceWrite (bool success, std::stack<short int> &succ, coord S, coord T) {
	std::ofstream OutFile ("tmp");
	if (success) {
		coord c = S;
		while (!succ.empty()) {
			OutFile << (c.layer)+1 << " " << c.x << " " << c.y << endl;
			short int tmp = -succ.top();
			if ((tmp==UP) || (tmp==DOWN)) {
				OutFile << "3" << " " << c.x << " " << c.y << endl;
			}
			pointingCoord (tmp, c);	
			succ.pop();
		}
		OutFile << (c.layer)+1 << " " << c.x << " " << c.y << endl;
	}
	OutFile << "0" << endl;
	OutFile.close();
}

void netRoute (char* name, int nx, int ny, int nl, int bendPenalty, int viaPenalty, std::vector<std::vector<std::vector<short int> > > &gridCost, std::vector<std::vector<int> > &netlist) {
	std::vector<std::vector<std::vector<gridInfo_t> > > gridInfo;
	gridInfo.resize(2, std::vector< std::vector<gridInfo_t> >(ny, std::vector<gridInfo_t>(nx)));
	bool success;
	std::stack<short int> succ;
	
	std::ofstream OutFile ("out");
	
	OutFile << nl << endl;
		
	for (int i=0; i<nl; i++) {
		OutFile << i+1 << endl;
		coord S, T;
		S.layer = netlist[i][0];
		S.x = netlist[i][1];
		S.y = netlist[i][2];
		T.layer = netlist[i][3];
		T.x = netlist[i][4];
		T.y = netlist[i][5];
		cout << "** Netlist-" << i+1 << "\t:\t" << S.layer << "\t" << S.x << "\t" << S.y << "\t" << T.layer << "\t" << T.x << "\t" << T.y << endl;
		mazeRouter (S, T, nx, ny, bendPenalty, viaPenalty, gridCost, gridInfo, success, succ);
		///////// *** Writing to out file *** /////////
		if (success) {
			coord c = S;
			while (!succ.empty()) {
				OutFile << (c.layer)+1 << " " << c.x << " " << c.y << endl;
				short int tmp = -succ.top();
				if ((tmp==UP) || (tmp==DOWN)) {
					OutFile << "3" << " " << c.x << " " << c.y << endl;
				}
				pointingCoord (tmp, c);	
				succ.pop();
			}
			OutFile << (c.layer)+1 << " " << c.x << " " << c.y << endl;
		}
		OutFile << "0" << endl;
		///////////////////////////////////////////////
	}
	OutFile.close();
}

int main(int argc, char* argv[]) {
	int nx, ny, bendPenalty, viaPenalty, nNetlist;
	std::vector<std::vector<std::vector<short int> > > gridCost;
	std::vector<std::vector<int> > netlist; 
	
	readGrid (argv[1], &nx, &ny, &bendPenalty, &viaPenalty, gridCost);
	readNetlist (argv[2], &nNetlist, netlist);
	
	netRoute (argv[2], nx, ny, nNetlist, bendPenalty, viaPenalty, gridCost, netlist);

}
