if [ "$1" == "simple parallel" ]; then
    ./3d_scene "./" "simple_scene.obj" "8.94211 5.5 -0.641418" "-0.951188 -0.293211 -0.0962711" "0.005" "true" "parallel" "6.9383 4.5 -1.58225" "-0.981657 -0.1906 0.00459649" "10.0 0.1 50.0"
elif [ "$1" == "simple point" ]; then
    ./3d_scene "./" "simple_scene.obj" "8.94211 5.5 -0.641418" "-0.951188 -0.293211 -0.0962711" "0.002" "true" "point" "6.64111 6 0.705563" "-0.878448 -0.472993 -0.0678688" "45.0 0.1 100.0"
elif [ "$1" == "wind" ]; then
    ./3d_scene "./windmill/" "Windmill.obj" "0.709 132.5 32.9817" "0.00297343 -0.95525 -0.295785" "0.005" "false" "parallel" "0.61623 132.5 85.4783" "0.00325378 -0.702065 -0.712105" "50 0.1 300.0"
fi
