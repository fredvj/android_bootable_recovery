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

    // fprintf(stderr, "screen_density := %d\n", screen_density);
    // fprintf(stderr, "min_x_swipe_px := %d\n", min_x_swipe_px);
    // fprintf(stderr, "min_y_swipe_px := %d\n", min_y_swipe_px);
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
    int abs_store[6] = {0};
    int k;
    set_min_swipe_lengths();

    // fprintf(stderr, "ev->type  := %d\n", ev->type);
    // fprintf(stderr, "ev->code  := %d\n", ev->code);

    if(ev->type == EV_ABS && ev->code == ABS_MT_TRACKING_ID) {
	fprintf(stderr, "ABS_MT_TRACKING_ID\n");
	fprintf(stderr, "ev->value := %d\n", ev->value);

	if(old_usec != 0L) {
	    if((ev->time.tv_sec*1000000L + ev->time.tv_usec) > (old_usec + 200000L)) {
		reset_gestures();
	    }
	}

	old_usec = ev->time.tv_sec*1000000L + ev->time.tv_usec;

    } else if(ev->type == EV_ABS && ev->code == ABS_MT_POSITION_X) {
	fprintf(stderr, "ABS_MT_POSITION_X\n");

	// ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), abs_store);
	// int max_x_touch = abs_store[2];

        old_x = touch_x;
        // float touch_x_rel = (float)ev->value / (float)max_x_touch;
        // touch_x = touch_x_rel * gr_fb_width();
	touch_x = ev->value;

        if(old_x != 0) diff_x += touch_x - old_x;

	fprintf(stderr, "value  := %d\n", ev->value);
	fprintf(stderr, "width  := %d\n", gr_fb_width());
	fprintf(stderr, "diffx  := %d\n", diff_x);

        if(diff_x > min_x_swipe_px) {
            ev->type = EV_KEY;
	    ev->code = KEY_POWER;
	    ev->value = 1;
            reset_gestures();
        } else if(diff_x < -min_x_swipe_px) {
            ev->type = EV_KEY;
 	    ev->code = KEY_BACK;
	    ev->value = 1;
            reset_gestures();
        }
    } else if(ev->type == EV_ABS && ev->code == ABS_MT_POSITION_Y) {
	fprintf(stderr, "ABS_MT_POSITION_Y\n");

	// ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), abs_store);
	// int max_y_touch = abs_store[2];

        old_y = touch_y;
        // float touch_y_rel = (float)ev->value / (float)max_y_touch;
        // touch_y = touch_y_rel * gr_fb_height();
	touch_y = ev->value;

        if(old_y != 0) diff_y += touch_y - old_y;

	fprintf(stderr, "value   := %d\n", ev->value);
	fprintf(stderr, "height  := %d\n", gr_fb_height());
	fprintf(stderr, "diff_y  := %d\n", diff_y);

        if(diff_y > min_y_swipe_px) {
            ev->type = EV_KEY;
            ev->code = KEY_VOLUMEDOWN;
	    ev->value = 1;
            reset_gestures();
        } else if(diff_y < -min_y_swipe_px) {
            ev->type = EV_KEY;
            ev->code = KEY_VOLUMEUP;
	    ev->value = 1;
            reset_gestures();
        }
    }

//	else if(ev->type == EV_ABS) {
//	fprintf(stderr, "EV_ABS with ev->code := 0x%x & ev->value := 0x%x\n", ev->code, ev->value);
//    } else {
//	fprintf(stderr, "ev->type := 0x%x & ev->code := 0x%x & ev->value := 0x%x\n", ev->type, ev->code, ev->value);
//    }

    return;
}
