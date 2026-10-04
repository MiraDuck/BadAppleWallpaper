#include "main.h"

int main(void) {
    printf("Starting wallpaper.\n");
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_MOUSE_PASSTHROUGH);
    SetTraceLogLevel(LOG_ERROR);

    InitWindow(800, 600, "Animated wallpaper");

    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    Vector2 monPos = GetMonitorPosition(monitor);

    void* hwnd = GetWindowHandle();
    reparent_to_workerw(hwnd, (int)monPos.x, (int)monPos.y, screenWidth, screenHeight);

    hide_taskbar_icon(hwnd);

    InitAudioDevice(); // once, near your other Init calls
    Music music = LoadMusicStreamFromMemory(".ogg", audio_ogg, audio_ogg_len);

    plm_video_t *vid = plm_video_create_with_buffer(plm_buffer_create_with_memory(video_m1v, video_m1v_len, 0), 0);

    double FPS = plm_video_get_framerate(vid);
    int vid_width = plm_video_get_width(vid);
    int vid_height = plm_video_get_height(vid);

    Color *pixelBuf = malloc(sizeof(Color) * vid_width * vid_height);

    plm_frame_t *frame;
    Texture2D texture = LoadTextureFromImage(GenImageColor(vid_width, vid_height, WHITE));

    float texturescale = (float)screenHeight/vid_height;
    Vector2 texture_pos = {(float)(screenWidth - ((float)texturescale * vid_width)) / 2, 0.0f};
    
    double videoTime = 0.0;
    double frameDuration = 1.0 / FPS;
    double audioTime = 0.0;

    start_keyboard_hook(); // to close the window
    float escHoldTime = 0.0f;
    const float HOLD_THRESHOLD = 1.5f;

    SetTargetFPS(FPS);
    PlayMusicStream(music);
    while (!WindowShouldClose()) {
        UpdateMusicStream(music);
        audioTime = GetMusicTimePlayed(music);

        do {
            frame = plm_video_decode(vid);
            if (frame == NULL) goto end;
            videoTime+= frameDuration;
        } while (audioTime > videoTime);

        for (int i = 0; i < vid_width * vid_height; i++) {
            uint8_t luma = frame->y.data[i];
            pixelBuf[i] = (luma > 100) ? (Color){0,0,0,0} : (Color){0,0,0,255};
        }
        if (frame->y.width != vid_width)
            printf("y: %d vs vid: %d\n", frame->y.width, vid_width);

        UpdateTexture(texture, pixelBuf);

        // exit loop
        if (is_escape_held()) {
            escHoldTime += GetFrameTime();
        } else {
            escHoldTime = 0.0f;
        }
        
        // after drawing, check if it's time to actually quit:
        if (escHoldTime >= HOLD_THRESHOLD) break; // exits your main loop
        
        BeginDrawing();
            ClearBackground(BLANK);
            DrawRectangle(0,0,texture_pos.x+1,screenHeight, BLACK);
            DrawRectangle(screenWidth - texture_pos.x,0,texture_pos.x+1,screenHeight, BLACK);
            DrawTextureEx(texture, texture_pos, 0, texturescale, BLACK);

            // inside BeginDrawing/EndDrawing, after your video draw:
            if (escHoldTime > 0.0f) {
                float progress = escHoldTime / HOLD_THRESHOLD; // 0.0 to 1.0+
                if (progress > 1.0f) progress = 1.0f;

                int cx = screenWidth - 40;
                int cy = 40;
                int radius = 20;

                DrawCircleSector((Vector2){cx, cy}, radius, -90, -90 + 360 * progress,
                            32, (Color){150,0,0,200}); // filling progress arc
                DrawCircleLines(cx, cy, radius, (Color){255,255,255,180}); // background ring
            }
        EndDrawing();
    }
    end:
    UnloadMusicStream(music);
    CloseAudioDevice();
    free(pixelBuf);
    CloseWindow();
    return 0;
}