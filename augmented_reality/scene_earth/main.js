var scene, camera, renderer, light;
var earthRotY = 0, moonRotY = 0;
var radY = 0, radZ = -0.3;
var moonDist = 70;
var earthRadius = 25;
var earthMesh, tmpMesh;
var moonMesh;
var positionHistory = [];
var lastPos, diffMove, lastEarthScale;
var ping = 0;

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

function initMoon() {
    var moonTexture = THREE.ImageUtils.loadTexture(moonBase64);
    moonTexture.minFilter = THREE.NearestFilter;
    var moonMaterial = new THREE.MeshLambertMaterial({
        map: moonTexture,
    });
    var moonGeometry =  new THREE.SphereGeometry(earthRadius * 0.273, 10, 10);
    moonMesh = new THREE.Mesh(moonGeometry, moonMaterial);
    moonMesh.receiveShadow = true;
    moonMesh.castShadow = true;
    scene.add(moonMesh);
}

// Update position of objects in the scene
function update() {
    earthRotY += 0.007;
    earthMesh.rotation.y = earthRotY;

    // Update Moon position
    moonRotY += 0.005;
    radY += 0.03;
    radZ += 0.0005;

    // Calculate position on a sphere
    x = moonDist * Math.cos(radZ) * Math.sin(radY);
    y = moonDist * Math.sin(radZ) * Math.sin(radY);
    z = moonDist * Math.cos(radY);

    // We can keep `z` as is because we're not moving the Earth
    // along z axis.
    moonMesh.position.set(x + earthMesh.position.x, y + earthMesh.position.y, z);
    moonMesh.rotation.y = moonRotY;
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
    initMoon();
    initLight();
    // Start rendering the scene
    requestAnimationFrame(render);
});