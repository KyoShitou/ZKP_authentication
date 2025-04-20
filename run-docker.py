import argparse
import subprocess
import os

IMAGE_NAME = "zkp_host_img"
DOCKERFILE_DIR = "."  # Assumes Dockerfile is in the current directory

def build_image():
    print("Rebuilding Docker image...")
    subprocess.run(["docker", "build", "-t", IMAGE_NAME, DOCKERFILE_DIR], check=True)

def run_container():
    current_dir = os.getcwd()
    print(f"Running container from {IMAGE_NAME}...")
    subprocess.run([
        "docker", "run", "-it", "--rm",
        "--privileged",
        "-v", f"{current_dir}:/root/app",
        "-w", "/root/app",
        IMAGE_NAME
    ], check=True)

def main():
    parser = argparse.ArgumentParser(description="Run the zkp_host_img Docker container.")
    parser.add_argument('--rebuild', action='store_true', help='Rebuild the Docker image before running')
    args = parser.parse_args()

    if args.rebuild:
        build_image()
    run_container()

if __name__ == "__main__":
    main()
