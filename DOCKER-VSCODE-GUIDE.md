# Install Dev Containers

VSCode + Dev Containers is the preferred way to develop this project. Please look at [the official documentation](https://code.visualstudio.com/docs/devcontainers/tutorial) to learn how to install everything you need.

Once installed, the environment is configured via [`devcontainer.json`](.devcontainer/devcontainer.json) file. When opening the workspace, VSCode will ask you to reopen it in a container. Click "Yes," and you should be ready to start coding after a while! (the first time will be slower because the container needs to be generated).


# DEPRECATED (use only for reference)

1. Build the image
    ```
    docker build -t simplermenu_plus .
    ```
1. Run the image, binding current directory to `/work` directory inside the container.
    ```
    docker run -it --rm -v `pwd`:/work \
    -v `pwd`/resources/config/x86/.simplemenu:/userdata/system/.simplemenu \
    -v `pwd`/output/resources/menu.json:/root/menu.json simplermenu_plus:latest
    ```

1. Attach the running container to VSCode.
  
    1. Open _Remote Explorer_ from the vertical menu bar on the left and you should see the running docker container.
    
    1. If you don't see any container, add your user to the docker group by using a terminal to run: `sudo usermod -aG docker $USER`
    
    1. Click on the _Attach in New Window_ icon next to container's name to open a new VSCode instance attached to the container.

#### How to share docker display with the host machine

Not needed when using VSCode integration.

##### Linux
docker run -it --rm -e DISPLAY=${DISPLAY} --net host -v /tmp/.X11-unix/:/tmp/.X11-unix/:ro simplermenu_plus:latest output/simplermenu_plus

