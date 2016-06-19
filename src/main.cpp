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

enum directions {TOP, BOTTOM, LEFT, RIGHT, none};

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

  cell(){}
  cell(int color) {
    this->color = color;
  }
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

      if(y1 < 0 || y1 == grid.size()) continue;
      if(x1 < 0 || x1 == grid.size()) continue;
      
      int neighborColor = grid[x1][y1].color;
      if(neighborColor == 0 || neighborColor == color) okCells++;
    }
    if(okCells == 0) return true;
  }
  return false;
}

std::vector<std::vector<cell> > step(std::vector<std::vector<cell> > grid, int *currentColor, int x, int y) {
  iterCount++;
  if(y < 0 || y == grid.size()) return errorValue;
  if(x < 0 || x == grid.size()) return errorValue;

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

  grid[x][y].color = (*currentColor);

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

    // displayGrid(g);
    // std::cout << std::endl;
    if(!isImpossible(grid, indexes))
      solve(g, currentColor, x1, y1, indexes);
  }
  // std::cout << iterCount << std::endl;
  return grid;
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
  // std::vector<int> indexes = {0, 0, 1,
  // 			      4, 1, 1,
			      
  // 			      0, 3, 2,
  // 			      3, 1, 2,

  // 			      0, 4, 3,
  // 			      3, 2, 3,

  // 			      1, 3, 4,
  // 			      2, 2, 4,

  // 			      4, 2, 5,
  // 			      3, 4, 5};

  // auto grid = initGrid(5, indexes);

  // solve(grid, 1, 0, 0, indexes);
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


  solve(grid, 1, indexes[0], indexes[1], indexes);


  // std::cout <<  << std::endl;  
  return 0;

}
