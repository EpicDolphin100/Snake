#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>
#include <cstdlib>
#include <fcntl.h> // For fcntl()
#include <unistd.h> // For read(),
#include <string>
using namespace std;

void setNonBlockingInput() {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void renderScreen(int height, int width, int screen[20][20], string assets[4], int score, string blocks) {
  string result = "";
  system("clear");
  cout << blocks << endl;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      result += assets[screen[y][x]];
    }
    cout << "⬜" << result << "⬜" << endl;
    result = "";
  }
  cout << blocks << endl;
  cout << "Score: " << score << endl;
}

int kill(int score) {
  system("clear");
  cout << "GAME OVER!" << endl << endl << "Your score: " << score << endl;
  return 0;
}

int main() {
  const int height = 20;
  const int width = 20;
  int screen[height][width] = {
    0
  };

  // Initialize game assets
  string assets[4] = {
    "  ",
    "🟩",
    "🟨",
    "🍎"
  };

  //Border
  string block = "⬜";
  string border;

  for (int i = 0; i < 22; ++i) {
    border += block;
  }

  // Initialize apple, snake head, tail and score
  int apple[2] = {
    rand() % width,
    rand() % height
  };
  int head[2] = {
    height / 2,
    width / 2
  };
  int tail[width*height][2] = {
    {(height / 2)-1,
      (width / 2)-1
    }
  };
  int score = 0;

  //Place the Apple and Head
  screen[apple[1]][apple[0]] = 3;
  screen[head[0]][head[1]] = 2;

  //Configurations
  enableRawMode();
  setNonBlockingInput();

  //Initialize Input and Speed Config
  char c;
  int input = 4; // Start moving right
  auto start = chrono::high_resolution_clock::now();
  const int gameSpeed = 200; // 200 milliseconds per step

  while (true) {
    // Check if enough time has passed to update the game
    auto elapsed = chrono::duration_cast < chrono::milliseconds > (chrono::high_resolution_clock::now() - start).count();

    // Non-blocking read for keypress

    if (read(STDIN_FILENO, &c, 1) == 1) {
      if (c == 'q') break; // Quit game
      else if (c == 'w') input = 1; // Move up
      else if (c == 's') input = 2; // Move down
      else if (c == 'a') input = 3; // Move left
      else if (c == 'd') input = 4; // Move right
    }

    // Update snake's headition after gameSpeed milliseconds
    if (elapsed >= gameSpeed) {
      //Remove the head from screen and tails
      screen[head[0]][head[1]] = 0;
      for (auto i: tail) {
        screen[i[0]][i[1]] = 0; // Clear previous
      }
      //If collided with wall end game
      if (head[0] <= 0 || head[0] >= (height - 1) || head[1] <= 0 || head[1] >= (width - 1)) {
        return kill(score);
      }
      // Snake movement based on last input
      if (input == 1) head[0]--; // Up
      else if (input == 2) head[0]++; // Down
      else if (input == 3) head[1]--; // Left
      else if (input == 4) head[1]++; // Right

//Add apples
      if (screen[head[0]][head[1]] == 3) {
        score++;
        apple[0] = rand() % width;
        apple[1] = rand() % height;
        screen[apple[1]][apple[0]] = 3;
      }

      //Check if game over
      for(auto i: tail) {
        if (head[0] == i[0] && head[1] == i[1]) {
          return kill(score);
        }
      }
      //Update the tail
      for (int p = score; p >= 0; p--) {
        if (p == 0) {
          tail[p][0] = head[0];
          tail[p][1] = head[1];
        }
        else {
          tail[p][0] = tail[p-1][0];
          tail[p][1] = tail[p-1][1];
        }
      }
      for (int r = score; r >= 0; r--) {
        screen[tail[r][0]][tail[r][1]] = 1;
      }
      //Add head
      screen[head[0]][head[1]] = 2;
      // Render updated screen
      renderScreen(height, width, screen, assets, score, border);

      // Restart timer
      start = chrono::high_resolution_clock::now();
    }
  }

  return 0;
}