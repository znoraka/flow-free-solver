#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <ctime>
#include <map>
#include <set>
#include <regex>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

enum directions {TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3, none = 4};
std::vector<directions> directionsVector = {TOP, BOTTOM, LEFT, RIGHT};
std::vector<std::string> pipes = {"┐", "┌", "|", "─", "└", "┘"};
std::vector<std::vector<int> > filteredPipes;
std::vector<std::vector<directions> > filteredDirections;

int iterCount = 0;

void posFromDirection (int &x, int &y, directions d) {
  switch (d) {
  case TOP:    x--; return;
  case BOTTOM: x++; return;
  case LEFT:   y--; return;
  case RIGHT:  y++; return;
  case none: return;
  }
};


struct cell {
  int color = 0;
  bool node = false;
  bool done = false;
  int pipe = -1;

  void setColor(int color) {
    this->color = color;
    // this->pipe = color;
  }

  cell(){}
  cell(const cell& c) {
    this->color = c.color;
    this->node = c.node;
    this->done = c.done;
    this->pipe = c.pipe;
  }
  cell(int color) {
    this->color = color;
  }
};

std::vector<std::vector<cell> > errorValue;
std::vector<std::vector<cell> > resultGrid;

std::vector<std::vector<cell> > initGrid(std::vector<int> indexes);
std::vector<std::vector<cell> > step(std::vector<std::vector<cell> > grid, int *currentColor, int x, int y);
bool check(std::vector<std::vector<cell> > grid); 
void displayGrid(std::vector<std::vector<cell> > grid, bool pathes);
std::vector<std::vector<cell> > solve(std::vector<std::vector<cell> > grid, int currentColor, int x, int y, std::vector<int> indexes, int filled);
bool isImpossible(std::vector<std::vector<cell> > grid, std::vector<int> indexes);
int manhattanDistance(int x1, int y1, int x2, int y2);
bool nodesLinked(std::vector<std::vector<cell> > grid, int currentColor, std::vector<int> indexes, directions d, int startX, int startY, bool display);
std::vector<std::vector<cell> > solvePipes (std::vector<std::vector<cell> > grid1, int currentColor, int x, int y, std::vector<int> indexes, directions from, int lastPipe, int okNodes);
directions directionFromPipeAndDirection(int pipe, directions d);

std::vector<Vec3f> extractCircles(Mat src);
int getNumberOfCells(Mat src, std::vector<Vec3f> circles);
std::vector<int> indexesFromCircles(Mat src, std::vector<Vec3f> circles, int numberOfCells);

std::vector<std::vector<cell> > initGrid(int size, std::vector<int> indexes) {
  std::vector<std::vector<cell> > grid;
  grid.resize(size);
  for(auto &i : grid) {
    i.resize(size, cell());
  }

  for (int i = 0; i < indexes.size(); i += 3) {
    cell c(indexes[i + 2]);   
    c.node = true;
    c.done = (i % 2 == 0);
    grid[indexes[i]][indexes[i + 1]] = c;
  }
  return grid;
}

void displayGrid(std::vector<std::vector<cell> > grid, bool pathes = true) {
  if(!pathes) {
    for(auto i : grid) {
      for(auto j : i) {
	std::cout << j.color << " ";
      }
      std::cout << std::endl;
    }

    std::cout << std::endl;
  } else {
    std::string s;
    for(auto i : grid) {
      for(auto j : i) {
	if(j.node) s += std::to_string(j.color) + " ";
	else if(j.pipe == -1) s += ". ";
	else if(j.pipe == 9) s += "* ";
	else s += pipes[j.pipe] + " ";
      }
      s += "\n";
    }
    s = std::regex_replace(s, std::regex("─ "), "──");
    s = std::regex_replace(s, std::regex(" ─"), "──");
    s = std::regex_replace(s, std::regex("└ ┐"), "└─┐");
    s = std::regex_replace(s, std::regex("┌ ┘"), "┌─┘");
    s = std::regex_replace(s, std::regex("┌ "), "┌─");
    s = std::regex_replace(s, std::regex(" ┐"), "─┐");
    s = std::regex_replace(s, std::regex(" ┘"), "─┘");
    s = std::regex_replace(s, std::regex("└ "), "└─");
  

    std::cout << s << std::endl;
  }
}

//une configuration est mauvaise si un noeud est entouré par d'autres couleurs que la sienne ou zéro
bool isImpossible(std::vector<std::vector<cell> > grid, std::vector<int> indexes) {
  for (int i = 0; i < indexes.size(); i += 3) {
    int x = indexes[i + 0];
    int y = indexes[i + 1];
    int color = indexes[i + 2];
    
    int okCells = 0;
    for(int directionInt = TOP; directionInt != none; directionInt++) {
      directions d = static_cast<directions>(directionInt);
      int x1 = x, y1 = y;
      posFromDirection(x1, y1, d);

      if(y1 < 0 || y1 >= grid.size()) continue;
      if(x1 < 0 || x1 >= grid.size()) continue;
      
      int neighborColor = grid[x1][y1].color;
      if(neighborColor == 0 || neighborColor == color) okCells++;
    }
    if(okCells == 0) return true;
  }  
  return false;
}

std::vector<std::vector<cell> > step(std::vector<std::vector<cell> > grid, int *currentColor, int x, int y) {
  if(y < 0 || y >= grid.size()) return errorValue;
  if(x < 0 || x >= grid.size()) return errorValue;

  int tempCol = *currentColor;

  if(grid[x][y].color != 0) {
    if(grid[x][y].node && grid[x][y].color == (*currentColor) && !grid[x][y].done) {
      tempCol = 0;
      grid[x][y].done = true;
    } else {
      return errorValue;
    }
  }

  grid[x][y].setColor(*currentColor);

  (*currentColor) = tempCol;

  return grid;
}

bool check(std::vector<std::vector<cell> > grid) {
  for(auto i : grid) {
    for(auto j : i) {
      if(j.color == 0) return false;
    }
  }
  return true;
}

bool nodesLinked(std::vector<std::vector<cell> > grid, int currentColor, std::vector<int> indexes, directions d) {
  int startX = indexes[(currentColor - 1) * 6 + 0];
  int startY = indexes[(currentColor - 1) * 6 + 1];

  int endX = indexes[(currentColor - 1) * 6 + 3];
  int endY = indexes[(currentColor - 1) * 6 + 4];

  std::function<bool(int, int, directions)> f;
  f = [grid, indexes, endX, endY, &f](int x, int y, directions d) {
    posFromDirection(x, y, d);

    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;

    if(x == endX && y == endY) return true;

    directions nextD = d;
    int pipe = grid[x][y].pipe;

    if(pipe == -1) return false;

    nextD = directionFromPipeAndDirection(pipe, d);

    if(nextD == none) return false;

    return f(x, y, nextD);
  };

  return f(startX, startY, d);
}

std::vector<std::vector<cell> > solvePipes (std::vector<std::vector<cell> > grid1, int currentColor, int x, int y, std::vector<int> indexes, directions from, int lastPipe = 6, int okNodes = 0) {
    iterCount++;
    std::cout << x << ", " << y << std::endl;
    if(okNodes == indexes.size() / 6) {
      if(check(grid1)) {
	std::cout << "=================================" << std::endl;
	displayGrid(grid1);
	std::cout << "=================================" << std::endl;
	std::cout << iterCount << std::endl;
	exit(0);
	return grid1;
      } 
    }

  int endX = indexes[(currentColor - 1) * 6 + 3];
  int endY = indexes[(currentColor - 1) * 6 + 4];
    
  auto sortedFilteredDirections = filteredDirections[lastPipe];
  std::sort(sortedFilteredDirections.begin(), sortedFilteredDirections.end(), [x, y, endX, endY](directions d1, directions d2) {
      int x1 = x, x2 = x, y1 = y, y2 = y;
      posFromDirection(x1, y1, d1);
      posFromDirection(x2, y2, d2);
      
      return manhattanDistance(x1, y1, endX, endY) < manhattanDistance(x2, y2, endX, endY);
    });

  for(auto d : sortedFilteredDirections) {
    int x1 = x, y1 = y;
    posFromDirection(x1, y1, d);

    if(y1 < 0 || y1 >= grid1.size()) continue;
    if(x1 < 0 || x1 >= grid1.size()) continue;

    if(!isImpossible(grid1, indexes) && !grid1[x1][y1].node && grid1[x1][y1].pipe == -1 && grid1[x1][y1].color == currentColor) {
      /**** NE SELECTIONNER QUE LES PIPES QUI PERMETTENT D'ALLER DANS LA DIRECTION CHOISIE ****/
      /**** TRIER LES PIPES SELON LA DISTANCE DE MANHATTAN À LA TARGET POUR ACCELERER L'EXPLORATION ****/
      auto sortedFilteredPipes = filteredPipes[d];
      std::sort(sortedFilteredPipes.begin(), sortedFilteredPipes.end(), [d, x1, y1, endX, endY](int p1, int p2) {
	  int xa = x1, xb = x1, ya = y1, yb = y1;
	  posFromDirection(xa, ya, directionFromPipeAndDirection(p1, d));
	  posFromDirection(xb, yb, directionFromPipeAndDirection(p2, d));
      
	  return manhattanDistance(xa, ya, endX, endY) < manhattanDistance(xb, yb, endX, endY);
	});

      for(auto i : sortedFilteredPipes) {
	std::vector<std::vector<cell> > grid;
	for(auto i : grid1) {
	  grid.push_back(i);
	}
	int pipeTmp = i;

	grid[x1][y1].pipe = pipeTmp;
	directions tmp = from;
	int okNodesTmp = okNodes;
	if(tmp == none) tmp = d;
	int currentColorTmp = currentColor;

	if(nodesLinked(grid, currentColorTmp, indexes, tmp)) {
	  if(currentColor < indexes.size() / 6) {
	    x1 = indexes[(currentColor) * 6];
	    y1 = indexes[(currentColor) * 6 + 1];
	  }
	  currentColorTmp++;
	  okNodesTmp++;
	  pipeTmp = 6;
	  tmp = none;
	}
	solvePipes(grid, currentColorTmp, x1, y1, indexes, tmp, pipeTmp, okNodesTmp);
      }
    }
  }
  return grid1;
}

std::vector<std::vector<cell> > solve (std::vector<std::vector<cell> > grid, int currentColor, int x, int y, std::vector<int> indexes, int filled) {
  iterCount++;
  auto logDirection = [](directions d) {
    switch (d) {
    case TOP: std::cout << "TOP not possible" << std::endl; return;
    case BOTTOM: std::cout << "BOTTOM not possible" << std::endl; return;
    case LEFT: std::cout << "LEFT not possible" << std::endl; return;
    case RIGHT: std::cout << "RIGHT not possible" << std::endl; return;
    case none: std::cout << "none not possible" << std::endl; return;
    }
  };

  int endX = indexes[(currentColor - 1) * 6 + 3];
  int endY = indexes[(currentColor - 1) * 6 + 4];

  auto sortedFilteredDirections = directionsVector;
  std::sort(sortedFilteredDirections.begin(), sortedFilteredDirections.end(), [x, y, endX, endY](directions d1, directions d2) {
      int x1 = x, x2 = x, y1 = y, y2 = y;
      posFromDirection(x1, y1, d1);
      posFromDirection(x2, y2, d2);
      
      return manhattanDistance(x1, y1, endX, endY) < manhattanDistance(x2, y2, endX, endY);
    });
  
  for(auto d : sortedFilteredDirections) {
    int c = currentColor;
    int x1 = x, y1 = y;
    posFromDirection(x1, y1, d);
    
    std::vector<std::vector<cell> > g = step(grid, &c, x1, y1);

    if(g.size() == 0){
      continue;
    } 
      
    if(check(g)) {
      displayGrid(g, false);
      solvePipes(g, 1, indexes[0], indexes[1], indexes, none);
      resultGrid = g;
      return g;
    }

    if(c == 0) {
      x1 = indexes[(currentColor) * 6];
      y1 = indexes[(currentColor) * 6 + 1];
      currentColor++;
    }

    if(!isImpossible(grid, indexes) && currentColor <= indexes.size() / 6) {
      solve(g, currentColor, x1, y1, indexes, filled + 1);
    }
  }
  return grid;
}

directions directionFromPipeAndDirection(int pipe, directions d) {
  directions nextD = none;
  
  if(pipe == 0 && d == RIGHT)
    nextD = BOTTOM;
  else if(pipe == 0 && d == TOP)
    nextD = LEFT;

  else if(pipe == 1 && d == LEFT)
    nextD = BOTTOM;
  else if(pipe == 1 && d == TOP)
    nextD = RIGHT;

  else if(pipe == 2 && d == TOP)
    nextD = TOP;
  else if(pipe == 2 && d == BOTTOM)
    nextD = BOTTOM;

  else if(pipe == 3 && d == RIGHT)
    nextD = RIGHT;
  else if(pipe == 3 && d == LEFT)
    nextD = LEFT;

  else if(pipe == 4 && d == BOTTOM)
    nextD = RIGHT;
  else if(pipe == 4 && d == LEFT)
    nextD = TOP;

  else if(pipe == 5 && d == BOTTOM)
    nextD = LEFT;
  else if(pipe == 5 && d == RIGHT)
    nextD = TOP;

  return nextD;
}

int manhattanDistance(int x1, int y1, int x2, int y2) {
  return abs(x2 - x1) + abs(y2 - y1);
}


std::vector<Vec3f> extractCircles(Mat src) {
  Mat src_gray, src_eroded;

  for (int i = 0; i < src.rows; i++) {
    for (int j = 0; j < src.cols; j++) {
      Vec3b v = src.at<Vec3b>(i, j);
      if((v[0] + v[1] + v[2]) / 3 > 30) {
      	v[0] = 255;
      	v[1] = 255;
      	v[2] = 255;
      } else {
      	v[0] = 0;
      	v[1] = 0;
      	v[2] = 0;
      }
      src.at<Vec3b>(i, j) = v;
    }
  }

  erode(src, src, Mat());
  erode(src, src, Mat());
  
  dilate(src, src_eroded, Mat());
  dilate(src_eroded, src_eroded, Mat());
  dilate(src_eroded, src_eroded, Mat());

  for (int i = 0; i < src.rows; i++) {
    for (int j = 0; j < src.cols; j++) {
      Vec3b v = src.at<Vec3b>(i, j);
      Vec3b v1 = src_eroded.at<Vec3b>(i, j);
      if(v1[0] == v[0]) {
      	v[0] = 0;
      	v[1] = 0;
      	v[2] = 0;
      } else {
      	v[0] = 255;
      	v[1] = 255;
      	v[2] = 255;
      }
      src.at<Vec3b>(i, j) = v;
    }
  }

  cvtColor(src, src_gray, CV_RGB2GRAY);

  GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
  // GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
  std::vector<Vec3f> circles;

  HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.cols/20, 200, 40, 0, 0);
  for( size_t i = 0; i < circles.size(); i++ ) {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      // circle center
      circle( src, center, 3, Scalar(255,255,0), -1, 8, 0 );
      // circle outline
      circle( src, center, radius, Scalar(255,0,255), 3, 8, 0 );
   }
  imwrite("out.jpg", src);

  //TODO enlever les mauvais cercles
  return circles;
}

int getNumberOfCells(Mat src, std::vector<Vec3f> circles) {
  double minDistance = src.cols;
  
  for( size_t i = 0; i < circles.size(); i++ ) {
      Point c(cvRound(circles[i][0]), cvRound(circles[i][1]));
      for( size_t j = 0; j < circles.size(); j++ ) {
	if(j != i) {
	  Point c1(cvRound(circles[j][0]), cvRound(circles[j][1]));
	  double d = cv::norm(c - c1);
	  if(d < minDistance) minDistance = d;
	}
      }
   }

  return src.cols / minDistance;
}

std::vector<int> indexesFromCircles(Mat src, std::vector<Vec3f> circles, int numberOfCells) {
  std::set<std::pair<int, int> > couples;
  std::vector<int> indexes;
  
  for( size_t i = 0; i < circles.size(); i++ ) {
    Vec3b c1 = src.at<Vec3b>(cvRound(circles[i][1]), cvRound(circles[i][0]));
    std::cout << circles[i] << std::endl;
    std::cout << c1 << std::endl;
    int minDistance = 255 * 3;
    int index;
    
    for( size_t j = 0; j < circles.size(); j++ ) {
      if(i != j) {
	Vec3b c = src.at<Vec3b>(cvRound(circles[j][1]), cvRound(circles[j][0]));
	int distance = abs(c1[0] - c[0]) + abs(c1[1] - c[1]) + abs(c1[2] - c[2]);
	if(distance < minDistance) {
	  index = j;
	  minDistance = distance;
	  std::cout << minDistance << std::endl;
	}
      }
    }
    couples.insert(std::pair<int, int>(min(index, (int)i), max(index, (int)i)));
  }

  int n = 1;
  for(auto i : couples) {
    std::cout << i.first << " " << i.second << std::endl;
    indexes.push_back((circles[i.first][1] * numberOfCells) / src.cols);
    indexes.push_back((circles[i.first][0] * numberOfCells) / src.cols);
    indexes.push_back(n);

    indexes.push_back((circles[i.second][1] * numberOfCells) / src.cols);
    indexes.push_back((circles[i.second][0] * numberOfCells) / src.cols);
    indexes.push_back(n++);
  }
  return indexes;
}

int main(int argc, char **argv) {  
  filteredPipes.push_back({0, 1, 2}); //TOP
  filteredPipes.push_back({2, 4, 5}); //BOTTOM
  filteredPipes.push_back({1, 3, 4}); //LEFT
  filteredPipes.push_back({0, 3, 5}); //RIGHT

  filteredDirections.push_back({LEFT, BOTTOM});
  filteredDirections.push_back({RIGHT, BOTTOM});
  filteredDirections.push_back({TOP, BOTTOM});
  filteredDirections.push_back({LEFT, RIGHT});
  filteredDirections.push_back({TOP, RIGHT});
  filteredDirections.push_back({LEFT, TOP});
  filteredDirections.push_back({LEFT, TOP, RIGHT, BOTTOM});
  
  Mat img = imread(argv[1], 1);
  Mat img1 = imread(argv[1], 1);

  cv::resize(img, img, Size(400,400));
  cv::resize(img1, img1, Size(400,400));
  
  auto circles = extractCircles(img1);
  int numberOfCells = getNumberOfCells(img, circles);
  std::cout << "number of cells = " << numberOfCells << std::endl;
  auto indexes = indexesFromCircles(img, circles, numberOfCells);

  int n = 0;
  for(auto i : indexes) {
    std::cout << i << " ";
    n++;
    if(n == 3) std::cout << std::endl;
    if(n == 6) {
      std::cout << "\n" << std::endl;
      n = 0;
    } 

  }

  auto grid = initGrid(numberOfCells, indexes);
  displayGrid(grid);

  for(auto i : pipes) {
    std::cout << "pipe = " << i << std::endl;
  }

  solve(grid, 1, indexes[0], indexes[1], indexes, indexes.size() / 3);
  return 0;

}
