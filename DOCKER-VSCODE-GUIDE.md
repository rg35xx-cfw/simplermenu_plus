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

