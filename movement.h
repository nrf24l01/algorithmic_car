byte path_compiled[PATH_COMPILED_SIZE];
int path_point;

bool is_compiled(){
  bool compiled = true;
  for (int i = 0; i<PATH_COMPILED_SIZE; i=i+2){
    if ((path_compiled[i] == FORS) || (path_compiled[i] == FORE)) {
      compiled = false;
    }
  }
  return compiled;
}

bool iter_path_compile(){
  bool flag = true;
  int start_idx = -1;
  int finish_idx = -1;
  for (int i = 0; i<PATH_COMPILED_SIZE; i=i+2){
    if (path_compiled[i] == FORS){
      start_idx = i;
    }
    else if (path_compiled[i] == FORE){
      finish_idx = i;
      break;
    }
  }
  
  if ((start_idx == -1) || (finish_idx == -1)) {
    flag = false;
  }
  
  int count = path_compiled[finish_idx+1];
  int size = (finish_idx-start_idx-2);
  int compiled_size = size*count;
  int offset = (start_idx+1+compiled_size)-finish_idx-3;
  
  
  if (offset >= 0) {
    for (int i = PATH_COMPILED_SIZE-1; i>=finish_idx+2; i--){
      if (i+offset < PATH_COMPILED_SIZE){
        path_compiled[i+offset] = path_compiled[i];      
      }
      else if ((i+offset >= PATH_COMPILED_SIZE) & (path_compiled[i] != PASS)){
        flag = false;
      }
    }
  }
  else {
    for (int i = finish_idx+2; i<PATH_COMPILED_SIZE; i++){
      if (i+offset < PATH_COMPILED_SIZE){
        path_compiled[i+offset] = path_compiled[i];      
      }
      else if ((i+offset >= PATH_COMPILED_SIZE) & (path_compiled[i] != PASS)){
        flag = false;
      }
    }
  }

  
  for (int i = 0; i < size; i++){
    path_compiled[start_idx+i] = path_compiled[start_idx+2+i];
  }
  
  for (int i = 0; i < count; i++){
    for (int k = 0; k < size; k++){
      path_compiled[start_idx+k+size*i] = path_compiled[start_idx+k]; 
    }    
  }
  return flag;
}

bool compile_path(){
  bool compiled = true;
  for (int i = 0; i<PATH_COMPILED_SIZE; i++){
    if (i < PATH_SIZE){
      path_compiled[i] = path[i];
    }
    else {
      path_compiled[i] = 0;
    }
  }
  while (!(is_compiled())){
    if (!(iter_path_compile())){
      compiled = false;
      break;
    }
  }
  return compiled;
}


void setup_steppers(int speed = 4000, int acceleration = 1000){
  left_stepper.setRunMode(FOLLOW_POS);
  right_stepper.setRunMode(FOLLOW_POS);
  left_stepper.setMaxSpeed(speed);  
  left_stepper.setAcceleration(acceleration);
  left_stepper.autoPower(true);
  right_stepper.setMaxSpeed(speed);  
  right_stepper.setAcceleration(acceleration);
  right_stepper.autoPower(true);
  right_stepper.disable();
  left_stepper.disable();
}

bool move_tick(){
  bool lm = left_stepper.tick();
  bool rm = right_stepper.tick();
  return (lm || rm);
}

bool move(){
  if(!move_tick()){
    left_stepper.setCurrent(0);
    right_stepper.setCurrent(0);
    left_stepper.setTarget(0);
    right_stepper.setTarget(0);
    switch(path_compiled[path_point]) {
      case PASS:
        return true;
      case UP:
        left_stepper.setTarget(MOVS*path_compiled[path_point+1]);
        right_stepper.setTarget(MOVS*path_compiled[path_point+1]);
        break;
      case RIGHT:
        left_stepper.setTarget(ROTS*path_compiled[path_point+1]);
        right_stepper.setTarget(-1*ROTS*path_compiled[path_point+1]);
        break;
      case DOWN:
        left_stepper.setTarget(-1*MOVS*path_compiled[path_point+1]);
        right_stepper.setTarget(-1*MOVS*path_compiled[path_point+1]);
        break;
      case LEFT:
        left_stepper.setTarget(-1*ROTS*path_compiled[path_point+1]);
        right_stepper.setTarget(ROTS*path_compiled[path_point+1]);
        break;
    }
    path_point=path_point+2;
  }
  return false;
}