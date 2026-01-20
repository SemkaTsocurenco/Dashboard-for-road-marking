import QtQuick
import QtQuick3D

Node {
    id: root

    // Polynomial coefficients: x = a*y^2 + b*y + c
    property real polyA: 0
    property real polyB: 0
    property real polyC: 0

    // Y range for the line (in meters)
    property real yStartMeters: 0
    property real yEndMeters: 100

    // Styling
    property color lineColor: "#ffffff"
    property string lineStyle: "Solid"  // "Solid", "Dashed", "Double"

    // Line width (match RoadPlane edge line width)
    property real lineWidth: 0.15
    property real lineHeight: 0.02

    // Number of segments
    property int segmentCount: 20

    // Force X as the independent variable (y = f(x)) instead of x = f(y).
    property bool swapXY: true

    // Optional points to clamp the curve range and infer axis orientation.
    property var points: []
    property bool usePointsRange: true
    property bool autoAxisFromPoints: true

    property real pointsMinX: {
        if (!points || points.length === 0)
            return 0;
        var min = Infinity;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            if (p.x < min)
                min = p.x;
        }
        return min === Infinity ? 0 : min;
    }

    property real pointsMaxX: {
        if (!points || points.length === 0)
            return 0;
        var max = -Infinity;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            if (p.x > max)
                max = p.x;
        }
        return max === -Infinity ? 0 : max;
    }

    property real pointsMinY: {
        if (!points || points.length === 0)
            return 0;
        var min = Infinity;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            if (p.y < min)
                min = p.y;
        }
        return min === Infinity ? 0 : min;
    }

    property real pointsMaxY: {
        if (!points || points.length === 0)
            return 0;
        var max = -Infinity;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            if (p.y > max)
                max = p.y;
        }
        return max === -Infinity ? 0 : max;
    }

    property real pointsRangeX: pointsMaxX - pointsMinX
    property real pointsRangeY: pointsMaxY - pointsMinY

    property bool hasValidPoints: {
        if (!points || points.length < 2)
            return false;
        return Math.abs(pointsRangeX) > 0.001 || Math.abs(pointsRangeY) > 0.001;
    }

    property real errorXFromY: {
        if (!hasValidPoints)
            return 0;
        var sum = 0;
        var count = 0;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            var pred = polyA * p.y * p.y + polyB * p.y + polyC;
            if (!isFinite(pred))
                continue;
            var diff = pred - p.x;
            sum += diff * diff;
            count++;
        }
        return count > 0 ? sum : Infinity;
    }

    property real errorYFromX: {
        if (!hasValidPoints)
            return 0;
        var sum = 0;
        var count = 0;
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue;
            var pred = polyA * p.x * p.x + polyB * p.x + polyC;
            if (!isFinite(pred))
                continue;
            var diff = pred - p.y;
            sum += diff * diff;
            count++;
        }
        return count > 0 ? sum : Infinity;
    }

    property bool useXasIndependent: {
        if (swapXY)
            return true;
        if (!autoAxisFromPoints || !hasValidPoints)
            return false;
        return errorYFromX < errorXFromY;
    }

    property real startCoord: {
        if (usePointsRange && hasValidPoints)
            return useXasIndependent ? pointsMinX : pointsMinY;
        return yStartMeters;
    }

    property real endCoord: {
        if (usePointsRange && hasValidPoints)
            return useXasIndependent ? pointsMaxX : pointsMaxY;
        return yEndMeters;
    }

    property real rangeCoord: endCoord - startCoord

    // Visibility
    property bool lineValid: true // Math.abs(yEndMeters - yStartMeters) > 0.1

    visible: lineValid

    // Generate line segments
    Repeater3D {
        id: lineRepeater
        model: root.lineValid ? root.segmentCount : 0

        Model {
            id: segment
            required property int index

            property real t: index / (root.segmentCount - 1)
            property real uPos: root.startCoord + t * root.rangeCoord
            property real vPos: root.polyA * uPos * uPos + root.polyB * uPos + root.polyC

            property real tNext: Math.min(1.0, (index + 1) / (root.segmentCount - 1))
            property real uNext: root.startCoord + tNext * root.rangeCoord
            property real vNext: root.polyA * uNext * uNext + root.polyB * uNext + root.polyC

            property real xPos: root.useXasIndependent ? uPos : vPos
            property real yPos: root.useXasIndependent ? vPos : uPos
            property real xNext: root.useXasIndependent ? uNext : vNext
            property real yNext: root.useXasIndependent ? vNext : uNext

            // Segment length and angle
            property real dx: xNext - xPos
            property real dy: yNext - yPos
            property real segmentLength: Math.sqrt(dx * dx + dy * dy)
            property real angle: Math.atan2(dx, dy) * 180 / Math.PI

            // Dashed line: show every other segment
            property bool isDashed: root.lineStyle === "Dashed"
            property bool showSegment: !isDashed || (index % 2 === 0)

            visible: showSegment

            source: "#Cube"

            position: Qt.vector3d(
                (xPos + dx / 2) * sceneConfig.scaleFactor,
                1.0,  // Match LanePoint height (slightly above road surface)
                (yPos + dy / 2) * sceneConfig.scaleFactor
            )

            scale: Qt.vector3d(
                root.lineWidth,
                root.lineHeight,
                segmentLength * sceneConfig.scaleFactor
            )

            eulerRotation: Qt.vector3d(0, angle, 0)

            materials: PrincipledMaterial {
                baseColor: root.lineColor
                roughness: 0.75
                specularAmount: 0.0
                metalness: 0.0
                emissiveFactor: Qt.vector3d(
                    root.lineColor.r * 0.5,
                    root.lineColor.g * 0.5,
                    root.lineColor.b * 0.5
                )
            }
        }
    }

    // Second line for Double style (offset to the side)
    Repeater3D {
        id: doubleLineRepeater
        model: (root.lineValid && root.lineStyle === "Double") ? root.segmentCount : 0

        Model {
            id: doubleSegment
            required property int index

            property real t: index / (root.segmentCount - 1)
            property real uPos: root.startCoord + t * root.rangeCoord
            property real vPos: root.polyA * uPos * uPos + root.polyB * uPos + root.polyC

            property real tNext: Math.min(1.0, (index + 1) / (root.segmentCount - 1))
            property real uNext: root.startCoord + tNext * root.rangeCoord
            property real vNext: root.polyA * uNext * uNext + root.polyB * uNext + root.polyC

            property real xPos: root.useXasIndependent ? uPos : vPos
            property real yPos: root.useXasIndependent ? vPos : uPos
            property real xNext: root.useXasIndependent ? uNext : vNext
            property real yNext: root.useXasIndependent ? vNext : uNext

            property real dx: xNext - xPos
            property real dy: yNext - yPos
            property real segmentLength: Math.sqrt(dx * dx + dy * dy)
            property real angle: Math.atan2(dx, dy) * 180 / Math.PI

            // Offset perpendicular to the line direction
            property real offsetAmount: 0.15
            property real perpX: segmentLength > 0.001 ? -dy / segmentLength * offsetAmount : 0
            property real perpY: segmentLength > 0.001 ? dx / segmentLength * offsetAmount : 0

            source: "#Cube"

            position: Qt.vector3d(
                (xPos + dx / 2 + perpX) * sceneConfig.scaleFactor,
                1.0,  // Match LanePoint height (slightly above road surface)
                (yPos + dy / 2 + perpY) * sceneConfig.scaleFactor
            )

            scale: Qt.vector3d(
                root.lineWidth * 0.8,
                root.lineHeight,
                segmentLength * sceneConfig.scaleFactor
            )

            eulerRotation: Qt.vector3d(0, angle, 0)

            materials: PrincipledMaterial {
                baseColor: root.lineColor
                roughness: 0.75
                specularAmount: 0.0
                metalness: 0.0
                emissiveFactor: Qt.vector3d(
                    root.lineColor.r * 0.5,
                    root.lineColor.g * 0.5,
                    root.lineColor.b * 0.5
                )
            }
        }
    }
}
