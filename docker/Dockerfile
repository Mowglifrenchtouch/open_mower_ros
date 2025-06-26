# ---------- Base Image ----------
FROM docker.io/ros:noetic-ros-base-focal AS base

ARG ROS_DISTRO=noetic
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install --yes \
    ccache \
    git \
    mosquitto \
    python3.8-venv \
    nginx && \
    rm -rf /var/www /etc/nginx/sites-enabled/* /var/lib/apt/lists/*

# Install custom configs
COPY --link ./docker/assets/mosquitto.conf /etc/mosquitto/mosquitto.conf
COPY --link ./docker/assets/nginx.conf /etc/nginx/conf.d/default.conf

# Update rosdep
RUN rosdep update --rosdistro $ROS_DISTRO


# ---------- slic3r build ----------
FROM base AS slic3r

# Copy slic3r package
COPY --link src/lib/slic3r_coverage_planner /opt/slic3r_coverage_planner_workspace/src

WORKDIR /opt/slic3r_coverage_planner_workspace

# Install slic3r dependencies
RUN rosdep install --from-paths src --ignore-src --simulate | \
    sed '1d' | sort | tr -d '\n' | sed 's/  apt-get install//g' > apt-install_list && \
    apt-get update && \
    apt-get install --no-install-recommends --yes $(cat apt-install_list) libasio-dev && \
    rm -rf /var/lib/apt/lists/* apt-install_list

# Build slic3r
RUN bash -c "source /opt/ros/$ROS_DISTRO/setup.bash && catkin_make"
RUN bash -c "source /opt/ros/$ROS_DISTRO/setup.bash && source devel/setup.bash && \
             catkin_make -DCMAKE_INSTALL_PREFIX=/opt/prebuilt/slic3r_coverage_planner install"


# ---------- Dependency resolver ----------
FROM base AS dependencies

WORKDIR /opt/open_mower_ros

# Generate list of dependencies
RUN --mount=type=bind,source=src,target=src \
    apt-get update && \
    rosdep install --from-paths src --ignore-src --simulate | \
    sed '1d' | sort | tr -d '\n' | sed 's/  apt-get install//g' > /apt-install_list && \
    rm -rf /var/lib/apt/lists/*


# ---------- Final image ----------
FROM base AS assemble

ARG ROS_DISTRO=noetic
ARG OM_VERSION=dev
ENV DEBIAN_FRONTEND=noninteractive

# Copy prebuilt slic3r
COPY --link --from=slic3r /opt/prebuilt/slic3r_coverage_planner /opt/prebuilt/slic3r_coverage_planner

# Copy repo
COPY --link ./ /opt/open_mower_ros

# Copy package list
COPY --link --from=dependencies /apt-install_list /apt-install_list

# Install all dependencies
RUN apt-get update && \
    apt-get install --no-install-recommends --yes $(cat /apt-install_list) libasio-dev && \
    rm -rf /var/lib/apt/lists/* /apt-install_list

# Cleanup prebuilt src (to avoid shadowing)
RUN rm -rf /opt/open_mower_ros/src/lib/slic3r_coverage_planner

WORKDIR /opt/open_mower_ros

# Build open_mower excluding slic3r
RUN bash -c "source /opt/ros/$ROS_DISTRO/setup.bash && \
             cd src && catkin_init_workspace && \
             cd .. && \
             source /opt/prebuilt/slic3r_coverage_planner/setup.bash && \
             catkin_make -DCATKIN_BLACKLIST_PACKAGES=slic3r_coverage_planner -j$(nproc)"

# Version file
RUN echo \"export OM_SOFTWARE_VERSION=${OM_VERSION}\" > version_info.env

# Entrypoint
COPY ./docker/openmower_entrypoint.sh /openmower_entrypoint.sh
RUN chmod +x /openmower_entrypoint.sh

ENTRYPOINT ["/openmower_entrypoint.sh"]
CMD ["bash", "-c", "service nginx start; service mosquitto start; roslaunch open_mower open_mower.launch --screen"]
