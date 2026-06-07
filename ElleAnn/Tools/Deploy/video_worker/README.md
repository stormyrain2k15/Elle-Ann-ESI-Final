# Elle-Ann Video Worker

External Python process that turns `video_jobs` rows into lip-synced MP4 files.
Decoupled from the C++ core so you can run this on a GPU box while the main
services run anywhere.

## Pipeline
```
text  ─► TTS (edge-tts → WAV)
              │
avatar ──────┤
              ▼
         Wav2Lip (lip-sync)
              │
              ▼
     optional GFPGAN face enhance
              │
              ▼
         ffmpeg mux
              │
              ▼
         /api/video/worker/complete
```

## Prerequisites
1. Python 3.10+ with `pip install -r requirements.txt`.
2. Wav2Lip repo cloned + weights downloaded. `WAV2LIP_CHECKPOINT` points at
   the `.pth` file, `WAV2LIP_INFER` at that repo's `inference.py`.
3. `ffmpeg` on PATH (or set `FFMPEG_BIN`).
4. (optional) GFPGAN for upscale — set `GFPGAN_SCRIPT`.
5. The C++ HTTP service is reachable at `ELLE_API_BASE` (default
   `http://localhost:8000`).

## Run
```bash
# Windows (from this folder)
set ELLE_API_BASE=http://localhost:8000
set WAV2LIP_CHECKPOINT=C:\Models\Wav2Lip\wav2lip_gan.pth
set WAV2LIP_INFER=C:\Repos\Wav2Lip\inference.py
python elle_video_worker.py
```

```bash
# Linux / WSL
export ELLE_API_BASE=http://your-elle-box:8000
export WAV2LIP_CHECKPOINT=/models/wav2lip_gan.pth
export WAV2LIP_INFER=/src/Wav2Lip/inference.py
python3 elle_video_worker.py
```

The worker polls forever. Ctrl+C to stop. Crashes on a single job don't kill
the worker — the next iteration keeps going and the failed job is marked
`failed` in `video_jobs` so the UI can show it.

## Testing without the full Wav2Lip stack
If you just want to verify the queue wiring end-to-end, stub the heavy
steps:
```python
def run_wav2lip(avatar, wav, out_mp4):
    subprocess.run([FFMPEG_BIN, "-y", "-loop", "1", "-i", str(avatar),
                    "-i", str(wav), "-shortest", "-vf", "scale=512:512",
                    "-pix_fmt", "yuv420p", str(out_mp4)], check=True)
```
This produces a static-image "talking head" so you can see jobs complete
without a GPU.

## API contract (worker side)
- `POST /api/video/worker/claim`               → atomic claim of next queued job
- `POST /api/video/worker/progress/{id}`       → progress int 0–100
- `POST /api/video/worker/complete/{id}`       → `{ "output_path": "..." }`
- `POST /api/video/worker/fail/{id}`           → `{ "error": "..." }`
