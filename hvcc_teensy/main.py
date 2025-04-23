# grrrr.org, 2025

import os
import shutil
import time
import jinja2
from typing import Optional
from pydantic import BaseModel

from hvcc.generators.copyright import copyright_manager
from hvcc.generators.filters import filter_max

from hvcc.interpreters.pd2hv.NotificationEnum import NotificationEnum
from hvcc.types.compiler import Generator, CompilerResp, CompilerNotif, CompilerMsg, ExternInfo
from hvcc.types.meta import Meta


class hvcc_teensy(Generator):
    """Generates a Teensy Audio compatible DSP object.
    """

    @classmethod
    def compile(
        cls,
        c_src_dir: str,
        out_dir: str,
        externs: ExternInfo,
        patch_name: Optional[str] = None,
        patch_meta: Meta = Meta(),
        num_input_channels: int = 0,
        num_output_channels: int = 0,
        copyright: Optional[str] = None,
        verbose: Optional[bool] = False
    ) -> CompilerResp:

        tick = time.time()

        out_dir = os.path.join(out_dir, "teensy")
        receiver_list = externs.parameters.inParam

        copyright = copyright_manager.get_copyright_for_c("grrrr.org, 2025")

        patch_name = patch_name or "heavy"
        ext_name = f"{patch_name}"
        struct_name = f"{patch_name}"

        # ensure that the output directory does not exist
        out_dir = os.path.abspath(out_dir)
        if os.path.exists(out_dir):
            shutil.rmtree(out_dir)

        # copy over generated C source files
        shutil.copytree(c_src_dir, out_dir)

        # copy over static files
        shutil.copy(
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "static", "HvAudioProcessor.hpp"),
            f"{out_dir}/")

        try:
            # initialise the jinja template environment
            env = jinja2.Environment()
            env.filters["max"] = filter_max
            env.loader = jinja2.FileSystemLoader(
                os.path.join(os.path.dirname(os.path.abspath(__file__)), "templates"))

            defines = ""
            try:
                external_meta = patch_meta.external
            except:
                external_meta = None

            if external_meta is not None:
                result = external_meta.get("OPENAUDIO")
                if result is not None:
                    defines = f"#define OPENAUDIO {result}"

            # generate Arduino-style source from template
            teensy_path = os.path.join(out_dir, f"{ext_name}.h")
            with open(teensy_path, "w") as f:
                f.write(env.get_template("HeavyTeensy.h").render(
                    name=patch_name,
                    struct_name=struct_name,
                    display_name=ext_name,
                    num_input_channels=num_input_channels,
                    num_output_channels=num_output_channels,
                    receivers=receiver_list,
                    copyright=copyright,
                    defines=defines,
                ))

            # generate Makefile from template
#            teensy_path = os.path.join(out_dir, "../Makefile")
#            with open(teensy_path, "w") as f:
#                f.write(env.get_template("Makefile").render(
#                    name=patch_name))

            return CompilerResp(
                stage="hvcc_teensy",
                in_dir=c_src_dir,
                out_dir=out_dir,
                out_file=os.path.basename(teensy_path),
                compile_time=time.time() - tick
            )

        except Exception as e:
            return CompilerResp(
                stage="hvcc_teensy",
                notifs=CompilerNotif(
                    has_error=True,
                    exception=e,
                    warnings=[],
                    errors=[CompilerMsg(
                        enum=NotificationEnum.ERROR_EXCEPTION,
                        message=str(e)
                    )]
                ),
                in_dir=c_src_dir,
                out_dir=out_dir,
                compile_time=time.time() - tick
            )
