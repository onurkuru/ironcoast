"""Record six deterministic boss encounters into a silent 30 fps comparison video."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import subprocess
import tempfile


def run(command, **kwargs):
    return subprocess.run(command, check=True, capture_output=True, text=True, **kwargs)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--seconds", type=int, default=12)
    parser.add_argument("--phase", type=int, choices=(1, 2), default=1)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    args.work_dir.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    binary = str(args.binary.resolve())
    with tempfile.TemporaryDirectory(prefix="boss-captures-", dir=args.work_dir) as temp:
        temp = Path(temp)

        def capture(stage):
            frames = temp / f"frames-{stage}"
            frames.mkdir()
            result = run([binary, "--assets", str(root / "assets"), "--boss-preview",
                          "--stage", str(stage), "--preview-phase", str(args.phase),
                          "--fast", "--frames", str(args.seconds * 60), "--record", str(frames),
                          "--record-every", "2", "--save", str(temp / f"save-{stage}.dat")], env=env)
            video = temp / f"boss-{stage}.mp4"
            run(["ffmpeg", "-v", "error", "-y", "-framerate", "30", "-i",
                 str(frames / "frame%05d.png"), "-vf", "scale=480:272:flags=neighbor",
                 "-c:v", "libx264", "-preset", "fast", "-crf", "17", "-pix_fmt", "yuv420p",
                 str(video)])
            for frame in frames.glob("frame*.png"):
                frame.unlink()
            print(result.stdout.strip(), flush=True)
            return video

        with ThreadPoolExecutor(max_workers=2) as executor:
            videos = list(executor.map(capture, range(1, 7)))
        inputs = [arg for video in videos for arg in ("-i", str(video))]
        run(["ffmpeg", "-v", "error", "-y", *inputs, "-filter_complex",
             "[0:v][1:v][2:v][3:v][4:v][5:v]xstack=inputs=6:"
             "layout=0_0|480_0|960_0|0_272|480_272|960_272[v]", "-map", "[v]",
             "-c:v", "libx264", "-crf", "18", "-preset", "fast", "-pix_fmt", "yuv420p",
             "-movflags", "+faststart", str(args.output)])
    print(args.output, flush=True)


if __name__ == "__main__":
    main()
