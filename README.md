# cadpy

Install without build isolation:
pip install --no-build-isolation -ve .

Install with auto rebuild on import
pip install --no-build-isolation -Ceditable.rebuild=true -ve .

Install debug build
pip install --no-build-isolation -ve . --config-settings=cmake.build-type="Debug"

Full all of the above!
pip install --no-build-isolation -C editable.rebuild=true -ve . --config-settings=cmake.build-type="Debug"
