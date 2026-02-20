Import("env")

import csv
from pathlib import Path


def _resolve(path: str) -> Path:
    project_dir = Path(env.subst("$PROJECT_DIR"))
    return (project_dir / path).resolve()


def _read_model_offset(partition_csv: Path):
    if not partition_csv.exists():
        print(f"[ESP_SR_M5Unified] partition file not found: {partition_csv}")
        return None

    with partition_csv.open("r", encoding="utf-8") as f:
        rows = csv.reader(f)
        for row in rows:
            if not row or row[0].strip().startswith("#"):
                continue
            if len(row) < 5:
                continue
            if row[0].strip() == "model":
                return row[3].strip()

    print("[ESP_SR_M5Unified] model partition is not defined in partition table")
    return None


def _flash_srmodels(source, target, env_):
    partition_rel = env_.GetProjectOption("board_build.partitions", "")
    if not partition_rel:
        print("[ESP_SR_M5Unified] skip: board_build.partitions is not set")
        return

    partition_csv = _resolve(partition_rel)
    model_bin = _resolve("srmodels.bin")

    if not model_bin.exists():
        print(f"[ESP_SR_M5Unified] skip: srmodels.bin not found at {model_bin}")
        return

    offset = _read_model_offset(partition_csv)
    if not offset:
        return

    upload_port = env_.subst("$UPLOAD_PORT")
    if not upload_port:
        upload_port = env_.AutodetectUploadPort()

    upload_speed = env_.subst("$UPLOAD_SPEED") or "460800"
    uploader = env_.subst("$UPLOADER")
    pythonexe = env_.subst("$PYTHONEXE")
    chip = env_.BoardConfig().get("build.mcu", "esp32")

    cmd = (
        f'"{pythonexe}" "{uploader}" --chip {chip} --port "{upload_port}" '
        f"--baud {upload_speed} write_flash {offset} \"{model_bin}\""
    )

    print(f"[ESP_SR_M5Unified] flashing srmodels.bin to offset {offset}")
    env_.Execute(cmd)


env.AddPostAction("upload", _flash_srmodels)
