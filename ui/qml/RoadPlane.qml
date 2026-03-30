import QtQuick
import QtQuick3D

Node {
    id: root

    property real widthMeters: sceneConfig.roadWidth
    property real lengthMeters: sceneConfig.roadLength
    property real thicknessMeters: sceneConfig.roadThickness

    readonly property real planeY: sceneConfig.roadYPosition
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real scaledLength: lengthMeters * scaleFactor

    // Road shifted forward: bus 5m from rear edge, 95m of road ahead
    readonly property real roadCenterZ: (lengthMeters / 2.0 - 5.0) * scaleFactor
    readonly property real roadStartZ: roadCenterZ - scaledLength / 2.0
    readonly property real roadEndZ:   roadCenterZ + scaledLength / 2.0

    // ─── Camera lens world position (at car nose, forward_offset_m) ──────────
    // Physical camera is mounted at the nose, looks FORWARD (+Z direction)
    readonly property real cameraLensZ: sceneConfig.fovForwardOffsetM * scaleFactor

    // ─── Grid config — aligned to clean 5m intervals from camera ─────────────
    readonly property real gridSpacingM: 5.0
    // Grid lines at: cameraLensZ, cameraLensZ + 5*SF, cameraLensZ + 10*SF ...
    // In meters from road start, aligned to gridSpacingM
    readonly property real gridFirstM: Math.ceil((roadStartZ / scaleFactor) / gridSpacingM) * gridSpacingM
    readonly property real gridLastM:  Math.floor((roadEndZ  / scaleFactor) / gridSpacingM) * gridSpacingM / 2
    readonly property int  gridLineCount: Math.round((gridLastM - gridFirstM) / gridSpacingM) + 1

    // ─── Camera FOV geometry (forward-looking, +Z direction) ──────────────────
    readonly property real fovPitchRad:  sceneConfig.fovPitchDeg      * Math.PI / 180.0
    readonly property real fovHalfHRad:  sceneConfig.fovHorizontalDeg * Math.PI / 360.0  // HFOV/2
    readonly property real fovHalfVRad:  sceneConfig.fovVerticalDeg   * Math.PI / 360.0  // VFOV/2

    // Nearest ground point visible from lens (bottom edge of image hits road)
    readonly property real fovNearFromLensM: sceneConfig.fovHeightAboveRoadM /
                                             Math.tan(fovPitchRad + fovHalfVRad)

    // FOV edges in world Z — camera looks in +Z → near is slightly ahead, far is further ahead
    readonly property real fovNearZ: cameraLensZ + fovNearFromLensM          * scaleFactor
    readonly property real fovFarZ:  cameraLensZ + sceneConfig.fovMaxRangeM  * scaleFactor

    // X half-widths at near/far (based on distance from lens)
    readonly property real xHalfNearM: fovNearFromLensM         * Math.tan(fovHalfHRad)
    readonly property real xHalfFarM:  sceneConfig.fovMaxRangeM * Math.tan(fovHalfHRad)

    // Diagonal edge geometry:
    //   dX > 0 (FOV widens toward far), dZ > 0 (extends forward)
    readonly property real fovEdgeDxWorld: (xHalfFarM - xHalfNearM) * scaleFactor
    readonly property real fovEdgeDzWorld: fovFarZ - fovNearZ                           // positive
    readonly property real fovEdgeLenM:    Math.sqrt(fovEdgeDxWorld * fovEdgeDxWorld +
                                                     fovEdgeDzWorld * fovEdgeDzWorld) / scaleFactor
    // atan2(dx, dz) with dz>0 → angle in 1st quadrant → cube rotates to open forward
    readonly property real fovEdgeAngle:   Math.atan2(fovEdgeDxWorld, fovEdgeDzWorld) * 180.0 / Math.PI

    readonly property real fovEdgeCenterZ:    (fovNearZ + fovFarZ) / 2.0
    readonly property real fovEdgeCenterXAbs: (xHalfNearM + xHalfFarM) / 2.0 * scaleFactor

    // ─── Crossing zone (between camera lens and far_m ahead of it) ───────────
    // near = nearest the camera can see (bottom of image), far = crossingZoneFarM from camera
    readonly property real crossingNearZ:   fovNearZ
    readonly property real crossingFarZ:    cameraLensZ + sceneConfig.crossingZoneFarM * scaleFactor
    readonly property real crossingCenterZ: (crossingNearZ + crossingFarZ) / 2.0
    readonly property real crossingWidthM:  sceneConfig.crossingZoneLeftM + sceneConfig.crossingZoneRightM
    readonly property real crossingDepthM:  Math.abs(crossingNearZ - crossingFarZ) / scaleFactor

    // Y above floor to avoid z-fighting
    readonly property real lineY:     planeY + 0.8
    readonly property real crossingY: planeY + 1.5
    readonly property real fovY:      planeY + 2.0

    // // ════════════════════════════════════════════════════════════════════════
    // // EXTENDED TERRAIN — wide ground beyond road edges, fills empty space
    // // ════════════════════════════════════════════════════════════════════════
    // Model {
    //     source: "#Cube"
    //     position: Qt.vector3d(0, root.planeY - 4, root.roadCenterZ)
    //     scale: Qt.vector3d(600.0, root.thicknessMeters * 0.1, root.lengthMeters * 2.5)
    //     materials: PrincipledMaterial {
    //         baseColor: "#030508"
    //         metalness: 0.0
    //         roughness: 1.0
    //         emissiveFactor: Qt.vector3d(0.003, 0.004, 0.007)
    //     }
    // }

    // ════════════════════════════════════════════════════════════════════════
    // BASE GROUND PLANE — shifted forward, bus near rear edge
    // ════════════════════════════════════════════════════════════════════════
    // Model {
    //     source: "#Cube"
    //     position: Qt.vector3d(0, root.planeY - 2, root.roadCenterZ)
    //     scale: Qt.vector3d(70.0, root.thicknessMeters * 0.3, root.lengthMeters)
    //     materials: PrincipledMaterial {
    //         baseColor: "#090c10"
    //         metalness: 0.0
    //         roughness: 1.0
    //         opacity: 0.4
    //         emissiveFactor: Qt.vector3d(0.008, 0.01, 0.02)
    //     }
    // }

    // ════════════════════════════════════════════════════════════════════════
    // GRID LINES — every 5m in world-Z space
    // ════════════════════════════════════════════════════════════════════════
    Repeater3D {
        model: root.gridLineCount

        Model {
            required property int index

            readonly property real distFromCameraM: ((root.gridFirstM + index * root.gridSpacingM) * root.scaleFactor -
                    root.cameraLensZ) / root.scaleFactor
            readonly property real worldZ: root.gridFirstM + index * root.gridSpacingM * root.scaleFactor

            source: "#Cube"
            position: Qt.vector3d(0, root.lineY, worldZ)
            scale: Qt.vector3d(70.0, 0.015, 0.04)
            materials: PrincipledMaterial {
                baseColor: "#1a2840"
                metalness: 0.0
                roughness: 1.0
                emissiveFactor: Qt.vector3d(0.04, 0.08, 0.16)
            }
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    // CROSSING ZONE — orange rectangle, marks where crossing detection happens
    // ════════════════════════════════════════════════════════════════════════

    // Semi-transparent fill
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, root.crossingY, root.crossingCenterZ)
        scale: Qt.vector3d(root.crossingWidthM, root.thicknessMeters * 0.5, root.crossingDepthM)
        materials: PrincipledMaterial {
            baseColor: Qt.rgba(1.0, 0.55, 0.0, 0.13)
            alphaMode: PrincipledMaterial.Blend
            cullMode: PrincipledMaterial.NoCulling
            emissiveFactor: Qt.vector3d(0.6, 0.28, 0.0)
            roughness: 1.0
        }
    }

    // Near boundary (closest to camera lens)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, root.crossingY + 0.5, root.crossingNearZ)
        scale: Qt.vector3d(root.crossingWidthM, 0.025, 0.06)
        materials: PrincipledMaterial { baseColor: "#ff8800"; roughness: 0.2; emissiveFactor: Qt.vector3d(1.0, 0.45, 0.0) }
    }

    // Far boundary
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, root.crossingY + 0.5, root.crossingFarZ)
        scale: Qt.vector3d(root.crossingWidthM, 0.025, 0.06)
        materials: PrincipledMaterial { baseColor: "#ff8800"; roughness: 0.2; emissiveFactor: Qt.vector3d(1.0, 0.45, 0.0) }
    }

    // Left side
    Model {
        source: "#Cube"
        position: Qt.vector3d( root.crossingWidthM / 2.0 * root.scaleFactor, root.crossingY + 0.5, root.crossingCenterZ)
        scale: Qt.vector3d(0.06, 0.025, root.crossingDepthM)
        materials: PrincipledMaterial { baseColor: "#ff8800"; roughness: 0.2; emissiveFactor: Qt.vector3d(1.0, 0.45, 0.0) }
    }

    // Right side
    Model {
        source: "#Cube"
        position: Qt.vector3d(-root.crossingWidthM / 2.0 * root.scaleFactor, root.crossingY + 0.5, root.crossingCenterZ)
        scale: Qt.vector3d(0.06, 0.025, root.crossingDepthM)
        materials: PrincipledMaterial { baseColor: "#ff8800"; roughness: 0.2; emissiveFactor: Qt.vector3d(1.0, 0.45, 0.0) }
    }

    // ════════════════════════════════════════════════════════════════════════
    // FOV TRAPEZOID OUTLINE — opens toward +Z (forward, widening away from car)
    // ════════════════════════════════════════════════════════════════════════

    // Near edge (narrowest — just ahead of camera lens)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, root.fovY, root.fovNearZ)
        scale: Qt.vector3d(root.xHalfNearM * 2.0, 0.025, 0.06)
        materials: PrincipledMaterial { baseColor: "#00ccff"; roughness: 0.15; emissiveFactor: Qt.vector3d(0.0, 0.6, 1.0) }
    }

    // Far edge (widest — at max detection range ahead of car)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, root.fovY, root.fovFarZ)
        scale: Qt.vector3d(root.xHalfFarM * 2.0, 0.025, 0.06)
        materials: PrincipledMaterial { baseColor: "#00ccff"; roughness: 0.15; emissiveFactor: Qt.vector3d(0.0, 0.6, 1.0) }
    }

    // Side A diagonal — positive X side
    Model {
        source: "#Cube"
        position: Qt.vector3d( root.fovEdgeCenterXAbs, root.fovY, root.fovEdgeCenterZ)
        scale: Qt.vector3d(0.08, 0.025, root.fovEdgeLenM)
        eulerRotation: Qt.vector3d(0, root.fovEdgeAngle, 0)
        materials: PrincipledMaterial { baseColor: "#00ccff"; roughness: 0.15; emissiveFactor: Qt.vector3d(0.0, 0.6, 1.0) }
    }

    // Side B diagonal — negative X side (mirrored)
    Model {
        source: "#Cube"
        position: Qt.vector3d(-root.fovEdgeCenterXAbs, root.fovY, root.fovEdgeCenterZ)
        scale: Qt.vector3d(0.08, 0.025, root.fovEdgeLenM)
        eulerRotation: Qt.vector3d(0, -root.fovEdgeAngle, 0)
        materials: PrincipledMaterial { baseColor: "#00ccff"; roughness: 0.15; emissiveFactor: Qt.vector3d(0.0, 0.6, 1.0) }
    }
}
