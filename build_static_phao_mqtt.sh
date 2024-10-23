#!/bin/bash

# Display usage message
usage() {
  echo "Usage: $0 --package-contact=contact]"
  exit 1
}

# Initialize variables
PACKAGE_CONTACT=""
dockerfile="ubuntu_24.04_paho_mqtt_build.dockerfile"

# Extract arguments and values
for arg in "$@"
do
  case $arg in
    --package-contact=*)
    PACKAGE_CONTACT="${arg#*=}"
    shift
    ;;
    --builder-dockerfile=*)
    PACKAGE_CONTACT="${arg#*=}"
    shift
    ;;
    *)
    echo "Error: Unknown argument."
    usage
    ;;
  esac
done

if [ -z "$PACKAGE_CONTACT" ]; then
  echo "Error: --package-contact is required."
  usage
fi

OUTPUT_DIR=/thirdparty

docker build --file ./$dockerfile --build-arg PACKAGE_CONTACT=$PACKAGE_CONTACT --build-arg OUTPUT_DIR=$OUTPUT_DIR --progress=plain -t package_builder .
if [ $? -ne 0 ]; then
  docker image rm -f package_builder
  echo -e "\e[1;31mFailed building dockerfile.\e[0m" 
  exit 1
fi
docker run --name package_builder -d package_builder
if [ $? == 0 ]; then
  docker cp package_builder:$OUTPUT_DIR .
  rm -rf .$OUTPUT_DIR/_CPack_Packages
fi
docker stop package_builder
docker rm package_builder
docker image rm -f package_builder

cd .$OUTPUT_DIR
for file in *.tar.gz; do tar -xzf "$file"; done
echo -e "Packages extracted."
echo -e "\e[1;34mFinished creating packages.\e[0m"