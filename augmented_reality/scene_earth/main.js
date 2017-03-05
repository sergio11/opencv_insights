var scene, camera, renderer, light, earthMesh, earthRotY = 0;

function initScene(width, height) {
    scene = new THREE.Scene();
    // Setup camera with 45 deg field of view and same aspect ratio
    camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 1000);
    // Set the camera to 400 units along `z` axis
    camera.position.set(0, 0, 400);

    renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setSize(width, height);
    renderer.shadowMap.enabled = true;
    document.body.appendChild(renderer.domElement);
}

function initLight() {
    light = new THREE.SpotLight(0xffffff);
    // Position the light slightly to a side to make shadows look better.
    light.position.set(400, 100, 1000);
    light.castShadow = true;
    scene.add(light);
}

function initEarth() {
    // Load Earth texture and create material from it
    var earthMaterial = new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture(earthBase64)
    });
    // Create a sphere 25 units in radius and 16 segments
    // both horizontally and vertically.
    var earthGeometry = new THREE.SphereGeometry(25, 16, 16);
    earthMesh = new THREE.Mesh(earthGeometry, earthMaterial);
    earthMesh.receiveShadow = true;
    earthMesh.castShadow = true;
    // Add Earth to the scene
    scene.add(earthMesh);
}

// Update position of objects in the scene
function update() {
    earthRotY += 0.007;
    earthMesh.rotation.y = earthRotY;
}

// Redraw entire scene
function render() {
    update();
    renderer.setClearColor(0x000000, 0);
    renderer.render(scene, camera);
    // Schedule another frame
    requestAnimationFrame(render);
}

document.addEventListener('DOMContentLoaded', function(e) {
    // Initialize everything and start rendering
    initScene(window.innerWidth, window.innerHeight);
    initEarth();
    initLight();
    // Start rendering the scene
    requestAnimationFrame(render);
});