#define USER_DELAY	200000L
#define INJECT_KEY(key) {ev->type = EV_KEY; ev->code = key; ev->value = 1;}

static int touch_x = 0;
static int touch_y = 0;
static int old_x = 0;
static int old_y = 0;
static int diff_x = 0;
static int diff_y = 0;
static int min_x_swipe_px = 100;
static int min_y_swipe_px = 80;
static long old_usec = 0L;

static void set_min_swipe_lengths() {
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.sf.lcd_density", value, "0");
    int screen_density = atoi(value);
    if(screen_density > 0) {
        min_x_swipe_px = (int)(0.5 * screen_density); // Roughly 0.5in
        min_y_swipe_px = (int)(0.3 * screen_density); // Roughly 0.3in
    }
}

static void reset_gestures() {
    diff_x = 0;
    diff_y = 0;
    old_x = 0;
    old_y = 0;
    touch_x = 0;
    touch_y = 0;

    ui_clear_key_queue();
}

void swipe_handle_input(int fd, struct input_event *ev) {
    set_min_swipe_lengths();

    if(ev->type == EV_ABS) {
	switch(ev->code) {
	    case ABS_MT_TRACKING_ID:
		// Check if there haven't been MT events for 200ms (USER_DELAY)
		// The user probably moved his finger to a different location

		if((old_usec != 0L) &&
	           (ev->time.tv_sec*1000000L + ev->time.tv_usec) > (old_usec + USER_DELAY)) {
		    reset_gestures();
	    	}

		old_usec = ev->time.tv_sec*1000000L + ev->time.tv_usec;
		break;

	    case ABS_MT_POSITION_X:
	        old_x = touch_x;
		touch_x = ev->value;

	        if(old_x != 0) diff_x += touch_x - old_x;

		// Swipe right

	        if(diff_x > min_x_swipe_px) {
		    INJECT_KEY(KEY_POWER);
	            reset_gestures();
	        }

		// Swipe left

		if(diff_x < -min_x_swipe_px) {
	 	    INJECT_KEY(KEY_BACK);
	            reset_gestures();
	        }
		break;

	    case ABS_MT_POSITION_Y:
	        old_y = touch_y;
		touch_y = ev->value;

	        if(old_y != 0) diff_y += touch_y - old_y;

		// Swipe down

	        if(diff_y > min_y_swipe_px) {
	            INJECT_KEY(KEY_VOLUMEDOWN);
	            reset_gestures();
	        }

		// Swipe up

		if(diff_y < -min_y_swipe_px) {
	            INJECT_KEY(KEY_VOLUMEUP);
	            reset_gestures();
	        }
		break;
	}
    } // ev->type == EV_ABS
}
