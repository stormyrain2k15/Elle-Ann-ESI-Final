from __future__ import annotations

import os
import sys
import time
import uuid
import signal
import shutil
import logging
import threading
import subprocess
from pathlib import Path
from typing import Optional

import requests

API_BASE         = os.environ.get("ELLE_API_BASE", "http://localhost:8000")
WAV2LIP_CKPT     = os.environ.get("WAV2LIP_CHECKPOINT", "")
WAV2LIP_INFER    = os.environ.get("WAV2LIP_INFER", "")
GFPGAN_SCRIPT    = os.environ.get("GFPGAN_SCRIPT", "")
FFMPEG_BIN       = os.environ.get("FFMPEG_BIN", "ffmpeg")
TTS_ENGINE       = os.environ.get("TTS_ENGINE", "edge-tts")
VOICE_NAME       = os.environ.get("VOICE_NAME", "en-US-JennyNeural")
WORK_DIR         = Path(os.environ.get("WORK_DIR", "./work")).resolve()
OUTPUT_DIR       = Path(os.environ.get("OUTPUT_DIR", "./output")).resolve()
POLL_INTERVAL    = float(os.environ.get("POLL_INTERVAL_SEC", "3"))
MIN_ARTIFACT_BYTES = int(os.environ.get("MIN_ARTIFACT_BYTES", "512"))

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("elle.video.worker")

_shutdown = threading.Event()

def _install_signal_handlers() -> None:
    def _handler(signum, _frame):
        name = {signal.SIGINT: "SIGINT", signal.SIGTERM: "SIGTERM"}.get(
            signum, f"signal {signum}")
        if _shutdown.is_set():
            log.warning("received %s again -- exiting hard", name)
            os._exit(130)
        log.info("received %s -- entering graceful shutdown", name)
        _shutdown.set()

    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            signal.signal(sig, _handler)
        except (ValueError, OSError):

            log.warning("could not install handler for %s", sig)

class ClaimValidationError(ValueError):
def _require_nonempty_str(body: dict, key: str) -> str:
    if key not in body:
        raise ClaimValidationError(f"claim response missing required field '{key}'")
    v = body[key]
    if not isinstance(v, str):
        raise ClaimValidationError(
            f"claim response field '{key}' has type {type(v).__name__}, expected str")
    if not v.strip():
        raise ClaimValidationError(f"claim response field '{key}' is empty")
    return v

def _validate_claim(body: dict) -> dict:
    if not isinstance(body, dict):
        raise ClaimValidationError(
            f"claim response is {type(body).__name__}, expected object")
    if "claimed" not in body or not isinstance(body["claimed"], bool):
        raise ClaimValidationError("claim response missing/typed bool 'claimed'")
    if not body["claimed"]:
        return {}
    job_id = _require_nonempty_str(body, "job_id")
    text   = _require_nonempty_str(body, "text")
    avatar = _require_nonempty_str(body, "avatar_path")
    return {"job_id": job_id, "text": text, "avatar_path": avatar}

def claim_job() -> Optional[dict]:
    r = requests.post(f"{API_BASE}/api/video/worker/claim", timeout=10)
    r.raise_for_status()
    try:
        body = r.json()
    except ValueError as ex:
        raise ClaimValidationError(f"claim response was not valid JSON: {ex}") from ex
    validated = _validate_claim(body)
    return validated or None

def post_progress(job_id: str, pct: int) -> None:
    try:
        requests.post(
            f"{API_BASE}/api/video/worker/progress/{job_id}",
            json={"progress": int(pct)},
            timeout=5,
        )
    except requests.RequestException as ex:
        log.warning("progress POST failed: %s", ex)

def post_complete(job_id: str, output_path: str) -> None:
    resp = requests.post(
        f"{API_BASE}/api/video/worker/complete/{job_id}",
        json={"output_path": output_path},
        timeout=10,
    )
    resp.raise_for_status()

def post_fail(job_id: str, err: str) -> None:
    try:
        requests.post(
            f"{API_BASE}/api/video/worker/fail/{job_id}",
            json={"error": err[:2000]},
            timeout=10,
        )
    except requests.RequestException:
        pass

def _verify_artifact(p: Path, label: str) -> None:
    if not p.exists():
        raise RuntimeError(f"{label} artifact missing: {p}")
    if not p.is_file():
        raise RuntimeError(f"{label} artifact is not a regular file: {p}")
    sz = p.stat().st_size
    if sz < MIN_ARTIFACT_BYTES:
        raise RuntimeError(
            f"{label} artifact too small ({sz} bytes < {MIN_ARTIFACT_BYTES}): {p}")

def _safe_subprocess_env() -> dict:
    keep = {"PATH", "PATHEXT", "SYSTEMROOT", "WINDIR", "TEMP", "TMP",
            "HOME", "USERPROFILE", "APPDATA", "LOCALAPPDATA",
            "CUDA_VISIBLE_DEVICES", "CUDA_HOME", "CUDNN_HOME",
            "PYTHONPATH", "PYTHONHOME",
            "LANG", "LC_ALL", "LC_CTYPE"}
    return {k: v for k, v in os.environ.items() if k in keep}

def _run(cmd, **kwargs) -> None:
    kwargs.setdefault("check", True)
    kwargs.setdefault("cwd", str(WORK_DIR))
    kwargs.setdefault("env", _safe_subprocess_env())
    subprocess.run(cmd, **kwargs)

def synth_tts(text: str, out_wav: Path) -> None:
    if TTS_ENGINE == "edge-tts":

        mp3 = out_wav.with_suffix(".mp3")
        _run(
            ["edge-tts", "--voice", VOICE_NAME, "--text", text, "--write-media", str(mp3)],
        )
        _run(
            [FFMPEG_BIN, "-y", "-i", str(mp3), "-ar", "16000", "-ac", "1", str(out_wav)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        mp3.unlink(missing_ok=True)
    else:
        import pyttsx3

        engine = pyttsx3.init()
        engine.save_to_file(text, str(out_wav))
        engine.runAndWait()

def run_wav2lip(avatar: Path, wav: Path, out_mp4: Path) -> None:
    if not WAV2LIP_CKPT or not Path(WAV2LIP_CKPT).exists():
        raise RuntimeError(f"WAV2LIP_CHECKPOINT not set or missing: {WAV2LIP_CKPT}")
    if not WAV2LIP_INFER or not Path(WAV2LIP_INFER).exists():
        raise RuntimeError(f"WAV2LIP_INFER not set or missing: {WAV2LIP_INFER}")

    cmd = [
        sys.executable, WAV2LIP_INFER,
        "--checkpoint_path", WAV2LIP_CKPT,
        "--face",            str(avatar),
        "--audio",           str(wav),
        "--outfile",         str(out_mp4),
        "--nosmooth",
    ]
    log.info("wav2lip: %s", " ".join(cmd))
    _run(cmd)

def run_gfpgan(in_mp4: Path, out_mp4: Path) -> None:
    if not GFPGAN_SCRIPT:
        shutil.move(str(in_mp4), str(out_mp4))
        return

    frames_dir = in_mp4.parent / (in_mp4.stem + "_frames")
    enhanced_dir = in_mp4.parent / (in_mp4.stem + "_enhanced")
    frames_dir.mkdir(exist_ok=True)
    enhanced_dir.mkdir(exist_ok=True)

    _run(
        [FFMPEG_BIN, "-y", "-i", str(in_mp4), str(frames_dir / "frame_%05d.png")],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    _run(
        [sys.executable, GFPGAN_SCRIPT,
         "-i", str(frames_dir), "-o", str(enhanced_dir), "-s", "2"],
    )

    _run(
        [FFMPEG_BIN, "-y",
         "-framerate", "25",
         "-i", str(enhanced_dir / "restored_imgs" / "frame_%05d.png"),
         "-i", str(in_mp4),
         "-c:v", "libx264", "-pix_fmt", "yuv420p",
         "-c:a", "aac", "-map", "0:v:0", "-map", "1:a:0",
         str(out_mp4)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    shutil.rmtree(frames_dir, ignore_errors=True)
    shutil.rmtree(enhanced_dir, ignore_errors=True)

def _check_shutdown(job_id: str) -> None:
    if _shutdown.is_set():
        raise RuntimeError(
            f"worker shutdown requested mid-job {job_id} -- failing for requeue")

def process_job(job: dict) -> None:
    job_id     = job["job_id"]
    text       = job["text"]
    avatar     = Path(job["avatar_path"]).resolve()
    log.info("claim job_id=%s text=%.60s avatar=%s", job_id, text, avatar)

    if not avatar.exists():
        raise RuntimeError(f"avatar path missing on this worker: {avatar}")
    if not avatar.is_file():
        raise RuntimeError(f"avatar path is not a regular file: {avatar}")
    if avatar.stat().st_size < MIN_ARTIFACT_BYTES:
        raise RuntimeError(f"avatar file suspiciously small: {avatar}")

    WORK_DIR.mkdir(parents=True, exist_ok=True)
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    scratch = WORK_DIR / f"job_{job_id}_{uuid.uuid4().hex[:6]}"
    scratch.mkdir()

    success = False
    try:
        post_progress(job_id, 5)

        wav      = scratch / "voice.wav"
        raw_mp4  = scratch / "raw.mp4"
        final_mp4 = OUTPUT_DIR / f"{job_id}.mp4"

        _check_shutdown(job_id)
        log.info("step 1/4: tts")
        synth_tts(text, wav)
        _verify_artifact(wav, "tts wav")
        post_progress(job_id, 25)

        _check_shutdown(job_id)
        log.info("step 2/4: wav2lip")
        run_wav2lip(avatar, wav, raw_mp4)
        _verify_artifact(raw_mp4, "wav2lip raw mp4")
        post_progress(job_id, 70)

        _check_shutdown(job_id)
        log.info("step 3/4: gfpgan+mux (%s)",
                 "enabled" if GFPGAN_SCRIPT else "mux only")
        run_gfpgan(raw_mp4, final_mp4)
        _verify_artifact(final_mp4, "final mp4")
        post_progress(job_id, 95)

        _check_shutdown(job_id)
        log.info("step 4/4: commit (%s)", final_mp4)
        post_complete(job_id, str(final_mp4))
        log.info("job %s DONE (%s)", job_id, final_mp4)
        success = True

    finally:

        if success:
            shutil.rmtree(scratch, ignore_errors=True)
        else:
            log.warning("keeping scratch for post-mortem: %s", scratch)

def main() -> int:
    _install_signal_handlers()
    log.info("Elle-Ann video worker starting. api_base=%s poll=%.1fs",
             API_BASE, POLL_INTERVAL)

    if not shutil.which(FFMPEG_BIN):
        log.error("ffmpeg not found on PATH (FFMPEG_BIN=%s)", FFMPEG_BIN)
        return 2
    if not WAV2LIP_CKPT or not Path(WAV2LIP_CKPT).exists():
        log.error("WAV2LIP_CHECKPOINT missing (%s) -- set the env var before running",
                  WAV2LIP_CKPT)
        return 2
    if not WAV2LIP_INFER or not Path(WAV2LIP_INFER).exists():
        log.error("WAV2LIP_INFER missing (%s)", WAV2LIP_INFER)
        return 2

    while not _shutdown.is_set():
        try:
            job = claim_job()
        except ClaimValidationError as ex:

            log.error("protocol violation on claim: %s", ex)
            if _shutdown.wait(POLL_INTERVAL * 4):
                break
            continue
        except requests.RequestException as ex:
            log.warning("claim failed: %s (retrying in %.0fs)", ex, POLL_INTERVAL)
            if _shutdown.wait(POLL_INTERVAL):
                break
            continue

        if not job:
            if _shutdown.wait(POLL_INTERVAL):
                break
            continue

        try:
            process_job(job)
        except subprocess.CalledProcessError as ex:
            log.exception("subprocess failed on job %s: %s", job.get("job_id"), ex)
            post_fail(job["job_id"], f"subprocess rc={ex.returncode}: {ex}")
        except Exception as ex:
            log.exception("job failed")
            post_fail(job.get("job_id", "?"), str(ex))

    log.info("graceful shutdown complete")
    return 0

if __name__ == "__main__":
    sys.exit(main() or 0)
