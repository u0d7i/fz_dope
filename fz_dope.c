#include <furi.h>
#include <dolphin/dolphin.h>
#include <gui/gui.h>
#include <power/power_service/power.h>
#include <storage/storage.h>

static void fz_dope_draw_callback(Canvas * canvas, void *ctx) {
    UNUSED(ctx);
    char stats_str[30];

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close(RECORD_DOLPHIN);

    snprintf(stats_str, sizeof(stats_str),"State: L:%hu/B:%lu/C:%lu", stats.level, stats.butthurt, stats.icounter);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 7, stats_str);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 13, 25, "RESET dolphin state?");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 51, "[ok] to reset and reboot");
    canvas_draw_str(canvas, 0, 63, "[back] to exit");
}

static void fz_dope_input_callback(InputEvent * input_event, void *ctx) {
    furi_assert(ctx);
    FuriMessageQueue *event_queue = ctx;

    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t fz_dope(void *p) {
    UNUSED(p);

    InputEvent event;
    FuriMessageQueue *event_queue =
	furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort *view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, fz_dope_draw_callback, NULL);
    view_port_input_callback_set(view_port, fz_dope_input_callback,
				 event_queue);

    Gui *gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while (1) {
	furi_check(furi_message_queue_get
		   (event_queue, &event, FuriWaitForever) == FuriStatusOk);

	if (event.key == InputKeyBack) {
	    break;
	}
	if (event.key == InputKeyOk) {
	    Storage *storage = furi_record_open(RECORD_STORAGE);
	    storage_simply_remove(storage, INT_PATH(".dolphin.state"));
	    furi_record_close(RECORD_STORAGE);
	    power_reboot(PowerBootModeNormal);
	}
    }

    furi_message_queue_free(event_queue);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
