#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <ctime>
#include <map>
#include <set>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

enum directions {TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3, none = 4};
std::vector<std::string> pipes = {"┐", "┌", "|", "−", "└", "┘"};
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
    // this->pipe = std::to_string(color);
    this->pipe = color;
  }

  cell(){}
  cell(const cell &c) {
    this->color = c.color;
    this->node = c.node;
    this->done = c.done;
    this->pipe = c.pipe;
  }
  cell(int color) {
    this->color = color;
    
    // this->pipe = std::to_string(color);
    // this->pipe = color;
  }

  // cell(int pipe) {
  //   this->pipe = pipe;
  // }
};

std::vector<std::vector<cell> > errorValue;

std::vector<std::vector<cell> > initGrid(std::vector<int> indexes);
std::vector<std::vector<cell> > step(std::vector<std::vector<cell> > grid, int *currentColor, int x, int y);
bool check(std::vector<std::vector<cell> > grid); 
void displayGrid(std::vector<std::vector<cell> > grid);
std::vector<std::vector<cell> > solve(std::vector<std::vector<cell> > grid, int currentColor, int x, int y, std::vector<int> indexes);
bool isImpossible(std::vector<std::vector<cell> > grid, std::vector<int> indexes);

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

void displayGrid(std::vector<std::vector<cell> > grid) {
   for(auto i : grid) {
    for(auto j : i) {
      std::cout << j.color << " ";
    }
    std::cout << std::endl;
  }
   
  for(auto i : grid) {
    for(auto j : i) {
      if(j.node) std::cout << j.color << " ";
      else if(j.pipe == -1) std::cout << "." << " ";
      else if(j.pipe == 9) std::cout << "*" << " ";
      else std::cout << pipes[j.pipe] << " ";
    }
    std::cout << std::endl;
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
  iterCount++;
  if(y < 0 || y >= grid.size()) return errorValue;
  if(x < 0 || x >= grid.size()) return errorValue;

  int tempCol = *currentColor;

  if(grid[x][y].color != 0) {
    if(grid[x][y].node && grid[x][y].color == (*currentColor) && !grid[x][y].done) {
      tempCol = 0;
      grid[x][y].done = true;
      // std::cout << "currentColor = " << *currentColor << std::endl;
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

std::vector<std::vector<cell> > solve (std::vector<std::vector<cell> > grid, int currentColor, int x, int y, std::vector<int> indexes) {
  auto logDirection = [](directions d) {
    switch (d) {
    case TOP: std::cout << "TOP not possible" << std::endl; return;
    case BOTTOM: std::cout << "BOTTOM not possible" << std::endl; return;
    case LEFT: std::cout << "LEFT not possible" << std::endl; return;
    case RIGHT: std::cout << "RIGHT not possible" << std::endl; return;
    case none: std::cout << "none not possible" << std::endl; return;
    }
  };

  for(int directionInt = TOP; directionInt != none; directionInt++) {
    directions d = static_cast<directions>(directionInt);
    // std::cout << d << std::endl;
    int c = currentColor;
    int x1 = x, y1 = y;
    posFromDirection(x1, y1, d);
    
    std::vector<std::vector<cell> > g = step(grid, &c, x1, y1);

    if(g.size() == 0){
      // logDirection(d);
      continue;
    } 
      
    if(check(g)) {
      std::cout << "=================================" << std::endl;
      displayGrid(g);
      std::cout << "=================================" << std::endl;
      std::cout << iterCount << std::endl;
      exit(0);
      return g;
    }

    if(c == 0) {
      x1 = indexes[(currentColor) * 6];
      y1 = indexes[(currentColor) * 6 + 1];

      // std::cout << "x1 = " << x1 << std::endl;
      // std::cout << "y1 = " << y1 << std::endl;

      currentColor++;
    }

    if(!isImpossible(grid, indexes) && currentColor <= indexes.size() / 6) {
      solve(g, currentColor, x1, y1, indexes);
    } else {
      // displayGrid(g);
      // std::cout << std::endl;
    }
  }
  // std::cout << iterCount << std::endl;
  return grid;
}

bool pipesStuck(std::vector<std::vector<cell> > grid) {
  auto leftOk = [](std::vector<std::vector<cell> > grid, int x, int y) {
    posFromDirection(x, y, LEFT);
    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;
    if(grid[x][y].color != 0) return true;
    int pipe = grid[x][y].pipe;
    // if(!pipe.compare("┐") || !pipe.compare("|") || !pipe.compare("┘"))
    if(pipe == 0 || pipe == 2 || pipe == 5)
      return false;
    return true;
  };

 auto rightOk = [](std::vector<std::vector<cell> > grid, int x, int y) {
    posFromDirection(x, y, RIGHT);
    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;
    // std::string pipe = grid[x][y].pipe;
    if(grid[x][y].color != 0) return true;
    int pipe = grid[x][y].pipe;
    // if(!pipe.compare("┌") || !pipe.compare("|") || !pipe.compare("└"))
    if(pipe == 1 || pipe == 2 || pipe == 4)
      return false;
    return true;
 };

  auto topOk = [](std::vector<std::vector<cell> > grid, int x, int y) {
    posFromDirection(x, y, TOP);
    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;
    // std::string pipe = grid[x][y].pipe;
    if(grid[x][y].color != 0) return true;
    int pipe = grid[x][y].pipe;
    // if(!pipe.compare("−") || !pipe.compare("└") || !pipe.compare("┘"))
    if(pipe == 3 || pipe == 4 || pipe == 5)
      return false;
    return true;
  };

  auto bottomOk = [](std::vector<std::vector<cell> > grid, int x, int y) {
    posFromDirection(x, y, BOTTOM);
    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;
    // std::string pipe = grid[x][y].pipe;
    if(grid[x][y].color != 0) return true;
    int pipe = grid[x][y].pipe;
    // if(!pipe.compare("−") || !pipe.compare("┐") || !pipe.compare("┌"))
    if(pipe == 3 || pipe == 0 || pipe == 1)
      return false;
    return true;
  };
  
  for (int i = 0; i < grid.size(); i++) {
    for (int j = 0; j < grid.size(); j++) {
      int pipe = grid[i][j].pipe;

      // "┐", "┌", "|", "−", "└", "┘"
	
      // if(pipe.compare("┐") == 0) {
      if(pipe == 0) {
	if(!(leftOk(grid, i, j) && bottomOk(grid, i, j)))
	  return true;
      }

      // if(pipe.compare("┌") == 0) {
      if(pipe == 1) {
	if(!(rightOk(grid, i, j) && bottomOk(grid, i, j)))
	  return true;
      }

      // if(pipe.compare("|") == 0) {
      if(pipe == 2) {
	if(!(topOk(grid, i, j) && bottomOk(grid, i, j)))
	  return true;
      }

      // if(pipe.compare("−") == 0) {
      if(pipe == 3) {
	if(!(leftOk(grid, i, j) && rightOk(grid, i, j)))
	  return true;
      }
      
      // if(pipe.compare("└") == 0) {
      if(pipe == 4) {
	if(!(rightOk(grid, i, j) && topOk(grid, i, j)))
	  return true;
      }
       
      // if(pipe.compare("┘") == 0) {
      if(pipe == 5) {
	if(!(leftOk(grid, i, j) && topOk(grid, i, j)))
	  return true;
      }
    }
  }
  return false;
}

bool nodesLinked(std::vector<std::vector<cell> > grid, int currentColor, std::vector<int> indexes, directions d, bool display = false) {
  int startX = indexes[(currentColor - 1) * 6 + 0];
  int startY = indexes[(currentColor - 1) * 6 + 1];

  int endX = indexes[(currentColor - 1) * 6 + 3];
  int endY = indexes[(currentColor - 1) * 6 + 4];

  std::cout << "coming from " << d << std::endl;

  std::function<bool(int, int, directions)> f;
  f = [grid, indexes, endX, endY, &f](int x, int y, directions d) {
    posFromDirection(x, y, d);

    if(y < 0 || y >= grid.size()) return false;
    if(x < 0 || x >= grid.size()) return false;

    if(x == endX && y == endY) return true;

    directions nextD = d;
    int pipe = grid[x][y].pipe;

    if(pipe == -1) return false;

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

    else return false;

    return f(x, y, nextD);
  };

  return f(startX, startY, d);
}

std::vector<std::vector<cell> > solvePipes (std::vector<std::vector<cell> > grid1, int currentColor, int x, int y, std::vector<int> indexes, directions from, int lastPipe = 6) {
  // for(int directionInt = TOP; directionInt != none; directionInt++) {
  //   directions d = static_cast<directions>(directionInt);
  for(auto d : filteredDirections[lastPipe]) {
    int x1 = x, y1 = y;
    posFromDirection(x1, y1, d);

    if(y1 < 0 || y1 >= grid1.size()) continue;
    if(x1 < 0 || x1 >= grid1.size()) continue;

    // if(grid1[x1][y1].node || grid1[x1][y1].pipe == -1 || grid1[x1][y1].color == 0) {
    //   displayGrid(grid1);

    //   std::cout << "filtered node " << x1 << ", " << y1 << std::endl;
    // }

    // std::cout << currentColor << std::endl;

    if(!grid1[x1][y1].node && grid1[x1][y1].pipe == -1 && grid1[x1][y1].color == 0) {
      /**** NE SELECTIONNER QUE LES PIPES QUI PERMETTENT D'ALLER DANS LA DIRECTION CHOISIE ****/
      for (int i = 0; i < filteredPipes[d].size(); i++) {
	std::vector<std::vector<cell> > grid;
	grid.resize(grid1.size());
	for (int i = 0; i < grid1.size(); i++) {
	  grid[i].resize(grid1.size());
	  for (int j = 0; j < grid1.size(); j++) {
	    cell c(grid1[i][j]);
	    grid[i][j] = c;
	  }
	}

	grid[x1][y1].pipe = filteredPipes[d][i];
	grid[x1][y1].color = currentColor;
	displayGrid(grid);
	// std::cout << x1 << ", " << y1 << std::endl;
	directions tmp = from;
	if(tmp == none) tmp = d;
    	std::cout << "linked = " << nodesLinked(grid, currentColor, indexes, tmp, true) << std::endl;
	std::cout << std::endl;
	// std::cout << "pipesStuck = " << pipesStuck(grid) << std::endl;
	// std::cout <<  std::endl;

	if(!pipesStuck(grid)) {
	  if(nodesLinked(grid, currentColor, indexes, tmp)) {
	    // std::cout << "good : " << std::endl;
	    // displayGrid(grid);
	    std::cout <<  std::endl;
	    tmp = none;
	    x1 = indexes[(currentColor) * 6];
	    y1 = indexes[(currentColor) * 6 + 1];
	    // std::cout << x1 << ", " << y1 << std::endl;
	    currentColor++;
	  } else
	    solvePipes(grid, currentColor, x1, y1, indexes, tmp, filteredPipes[d][i]);
	}
      }
    }
  }
  return grid1;
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

  /// Show your results
  // namedWindow( "Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE );
  // imshow( "Hough Circle Transform Demo", src );
  //
  imwrite("out.jpg", src);
  // waitKey(0);

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
// enum directions {TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3, none = 4};
// std::vector<std::string> pipes = {"┐", "┌", "|", "−", "└", "┘"};
// std::vector<std::vector<int> > filteredPipes;

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


  // solve(grid, 1, indexes[0], indexes[1], indexes);
  solvePipes(grid, 1, indexes[0], indexes[1], indexes, none);

  // std::cout <<  << std::endl;  
  return 0;

}
