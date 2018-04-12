#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define INIT_X 400
#define INIT_Y 400
#define ANIM_SPEED 4
#define FPS 60
#define DEFAULT_CLEAR al_map_rgb(240, 240, 240)
#define MAIN_FONT "ttf/Hack-Bold.ttf"
#define MAIN_FONT_CL al_map_rgb(0,0,0)
#define SMALL_FONT_CL al_map_rgb(20,20,20)

typedef struct waypoint {
  int time; // seconds
  int max_time;
  float ratio;
  float target;
  float current;
  const char * name;
  struct waypoint * next;
} waypoint;

waypoint * new_waypoint(int in_time, const char * in_name) {
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

void delete_waypoints(waypoint * to_delete) {
  assert(to_delete != NULL);
  waypoint * cur = to_delete;
  while(cur->next != NULL) {
    waypoint * prev = cur;
    cur = cur->next;
    free(prev);
  }
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
  ALLEGRO_DISPLAY * disp = NULL;
  int width = INIT_X;
  int height = INIT_Y;
  ALLEGRO_EVENT_QUEUE * ev_q = NULL;
  ALLEGRO_TIMER * fps_tim = NULL;
  ALLEGRO_TIMER * counter = NULL;
  ALLEGRO_FONT * font = NULL;
  ALLEGRO_FONT * small_font = NULL;
  waypoint * points = new_waypoint(4, "first");
  append_waypoint(points, new_waypoint(3, "second"));
  waypoint * cur = points;
  int total = get_total_time(points);
  int current = total;
  float total_ratio = 1.0;
  float total_current = 0.0;
  float total_target = 0.0;
  assert(al_init());
  assert(al_init_font_addon());
  assert(al_init_ttf_addon());
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
  bool redraw;
  while(1) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(ev_q, &ev);
    if(ev.type == ALLEGRO_EVENT_TIMER) {
      if(ev.timer.source == fps_tim)
        redraw = true;
      if(ev.timer.source == counter) {
        printf("Counter went off!\n");
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
      al_draw_arc(width/2, height/2, al_get_font_line_height(font)*2, 0, total_current, al_map_rgb(220, 190, 190), 50);
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
