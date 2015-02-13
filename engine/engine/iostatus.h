#ifndef INC_IO_STATUS_H_
#define INC_IO_STATUS_H_

class CIOStatus {

public:

  struct TMouse {
    int   screen_x;     
    int   screen_y;
    float normalized_x;       
    float normalized_y;
    int   dx;
    int   dy;

    int   prev_x;
    int   prev_y;
    void update();
  };
  const TMouse &getMouse() const { return mouse; }

  void setMouseCoords( int x, int y );

  CIOStatus();
  void update( float elapsed );

protected:
  TMouse    mouse;
};

extern CIOStatus io;

#endif