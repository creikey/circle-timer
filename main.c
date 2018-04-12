#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define INIT_X 400
#define INIT_Y 400
#define ANIM_SPEED 4
#define FPS 60
#define DEFAULT_CLEAR al_map_rgb(240, 240, 240)
#define MAIN_FONT "/opt/circle-timer/ttf/Hack-Bold.ttf"
#define MAIN_FONT_CL al_map_rgb(0,0,0)
#define SMALL_FONT_CL al_map_rgb(20,20,20)

typedef struct waypoint {
  int time; // seconds
  int max_time;
  float ratio;
  float target;
  float current;
  char * name;
  struct waypoint * next;
} waypoint;

waypoint * new_waypoint(int in_time, char * in_name) {
  waypoint * to_return = malloc(sizeof *to_return);
  to_return->time = in_time;
  to_return->max_time = in_time;
  to_return->ratio = 1;
  to_return->name = in_name;
  to_return->target = 0.0;
  to_return->current = 0.0;
  to_return->next = NULL;
  return to_return;
}

void append_waypoint(waypoint * to_append_to, waypoint * to_append) {
  assert(to_append_to != NULL);
  waypoint * cur = to_append_to;
  while(cur->next != NULL) {
    cur = cur->next;
  }
  cur->next = to_append;
  to_append->next = NULL;
}

void print_waypoints(waypoint * points) {
  assert(points);
  waypoint * cur = points;
  while(cur->next != NULL) {
    printf("(t: %d)(n: %s) | ", cur->time, cur->name);
    cur = cur->next;
  }
  printf("(t: %d)(n: %s)\n", cur->time, cur->name);
}

void delete_waypoints(waypoint * to_delete) {
  assert(to_delete != NULL);
  waypoint * cur = to_delete;
  while(cur->next != NULL) {
    waypoint * prev = cur;
    cur = cur->next;
    free(prev->name);
    free(prev);
  }
  free(cur->name);
  free(cur);
}

int get_total_time(waypoint * points) {
  assert(points != NULL);
  waypoint * cur = points;
  int total = 0;
  while(cur->next != NULL) {
    total += cur->max_time;
    cur = cur->next;
  }
  total += cur->max_time;
  return total;
}

waypoint * parse_file(const char * filename) {
  const char * first_name = "none";
  char * to_set = malloc(sizeof *to_set * strlen(first_name) + 1);
  strcpy(to_set, first_name);
  waypoint * to_return = new_waypoint(0, to_set);
  FILE * file = fopen(filename, "r");
  assert(file);

  const int max_len = 150;
  char * tmp;
  while(1) {
    /*char * name = calloc(max_len, sizeof *name);
    tmp = fgets(name, max_len, file);
    if(tmp == NULL)
      break;
    char * tmptime = calloc(max_len, sizeof *tmptime);
    tmp = fgets(tmptime, max_len, file);
    if(tmp == NULL)
      break;
    int minutes;
    int seconds;
    char test[5];
    int val = fscanf(file, "%s\n", test);
    printf("Test is %s\n", test);
    printf("Val is %d\n", val);
    assert(val == 2);*/
    char * line1 = calloc(max_len, sizeof *line1);
    char * line2 = calloc(max_len, sizeof *line2);
    tmp = fgets(line1, max_len, file);
    if(tmp == NULL) {
      printf("Done reading conf file...\n");
      break;
    }
    tmp = fgets(line2, max_len, file);
    if(tmp == NULL) {
      printf("Trailing conf string after %s\n", line1);
      break;
    }
    char * name = line1;
    int minutes;
    int seconds;
    int err = sscanf(line2, "%d:%d", &minutes, &seconds);
    assert(err == 2);
    append_waypoint(to_return, new_waypoint((minutes*60)+seconds, name));
    free(line2);
  }
  fclose(file);
  return to_return;
}

char * waypoint_str(waypoint * to_str) {
  const size_t max_size = 8;
  char * to_return = malloc(sizeof *to_return * max_size);
  memset(to_return, '\0', max_size);
  int minutes = to_str->time/60;
  int seconds = to_str->time % 60;
  snprintf(to_return, max_size, "%d:%d", minutes, seconds);
  return to_return;
}  


int main(int argc, char ** argv) {
  waypoint * points;
  if(argc <= 1) {
    printf("Please input a configuration filename\n");
    return -1;
  } else {
    points = parse_file(argv[1]);
    assert(points);
  }
  print_waypoints(points);
  ALLEGRO_DISPLAY * disp = NULL;
  int width = INIT_X;
  int height = INIT_Y;
  ALLEGRO_EVENT_QUEUE * ev_q = NULL;
  ALLEGRO_TIMER * fps_tim = NULL;
  ALLEGRO_TIMER * counter = NULL;
  ALLEGRO_FONT * font = NULL;
  ALLEGRO_FONT * small_font = NULL;
  waypoint * cur = points;
  int total = get_total_time(points);
  int current = total;
  float total_ratio = 1.0;
  float total_current = 0.0;
  float total_target = 0.0;
  assert(al_init());
  assert(al_init_font_addon());
  assert(al_init_ttf_addon());
  assert(al_install_keyboard());
  font = al_load_ttf_font(MAIN_FONT,100,0);
  small_font = al_load_ttf_font(MAIN_FONT, 40,0);
  assert(small_font);
  assert(font);
  fps_tim = al_create_timer(1.0 / FPS);
  assert(fps_tim);
  al_start_timer(fps_tim);
  counter = al_create_timer(1);
  assert(counter);
  al_start_timer(counter);
  al_start_timer(fps_tim);
  al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
  disp = al_create_display(INIT_X, INIT_Y);
  assert(disp);
  assert(al_acknowledge_resize(disp));
  ev_q = al_create_event_queue();
  assert(ev_q);
  al_register_event_source(ev_q, al_get_display_event_source(disp));
  al_register_event_source(ev_q, al_get_timer_event_source(fps_tim));
  al_register_event_source(ev_q, al_get_timer_event_source(counter));
  al_register_event_source(ev_q, al_get_keyboard_event_source());
  bool redraw;
  while(1) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(ev_q, &ev);
    if(ev.type == ALLEGRO_EVENT_TIMER) {
      if(ev.timer.source == fps_tim)
        redraw = true;
      if(ev.timer.source == counter) {
        current--;
        total_ratio = (float)current/(float)total;
        total_target = total_ratio*ALLEGRO_PI*2;
        if(cur->time <= 0) {
          if(cur->next == NULL) {
            break;
          }
          cur = cur->next;
          al_set_timer_count(counter, 0);
        }
        cur->time = cur->max_time - al_get_timer_count(counter);
        if(cur->time != 0) {
          cur->ratio = (float)cur->time/(float)cur->max_time;
        } else {
          cur->ratio = 0.0;
        }
        cur->target = cur->ratio*ALLEGRO_PI*2;
      }
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      printf("Key down!\n");
      if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
        printf("Skipping %s with %d:%d left...\n", cur->name, (cur->time)/60, (cur->time) % 60);
        al_set_timer_count(counter, cur->max_time);
      }
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
      al_acknowledge_resize(disp);
      width=ev.display.width;
      height=ev.display.height;
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      break;
    }
    if(redraw && al_is_event_queue_empty(ev_q)) {
      redraw = false;
      al_clear_to_color(DEFAULT_CLEAR);
      cur->current += (cur->target-cur->current)/ANIM_SPEED;
      total_current += (total_target-total_current)/ANIM_SPEED;
      al_draw_arc(width/2, height/2, al_get_font_line_height(font)*1.5, 0, cur->current, al_map_rgb(240, 190, 190), 80);
      al_draw_arc(width/2, height/2, al_get_font_line_height(font)*2, 0, total_current, al_map_rgb(190, 220, 190), 50);
      al_draw_text(font, MAIN_FONT_CL, width/2, (height/2)-al_get_font_line_height(font)/2, ALLEGRO_ALIGN_CENTRE, waypoint_str(cur));
      al_draw_text(small_font, SMALL_FONT_CL, width/2, (height/2)-(al_get_font_line_height(small_font)/2)+300, ALLEGRO_ALIGN_CENTRE, cur->name);
      al_flip_display();
    }
  }

  al_destroy_timer(fps_tim);
  al_destroy_display(disp);
  al_destroy_event_queue(ev_q);

  delete_waypoints(points);
  
  return 0;
}
