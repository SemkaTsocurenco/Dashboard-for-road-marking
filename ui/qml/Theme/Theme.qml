pragma Singleton
import QtQuick

QtObject {
    // ========================================
    // COLOR PALETTE (synchronized with dark.qss)
    // ========================================

    // Backgrounds
    readonly property color bgBase: "#15181c"
    readonly property color bgSurface1: "#1a1f25"
    readonly property color bgSurface2: "#20262d"
    readonly property color bgSurface3: "#242b33"
    readonly property color bgSurface4: "#2a3038"

    // Borders
    readonly property color border: "#313842"
    readonly property color borderHover: "#47515c"

    // Text
    readonly property color textPrimary: "#e6e9ed"
    readonly property color textSecondary: "#a5abb2"
    readonly property color textDisabled: "#6f757c"

    // Accent (cyan/teal)
    readonly property color accent: "#39b9c6"
    readonly property color accentHover: "#53cfdc"
    readonly property color accentPressed: "#2aa3b0"

    // Status colors
    readonly property color success: "#4dbb6b"
    readonly property color warning: "#e1b341"
    readonly property color danger: "#e15b5b"

    // Disabled states
    readonly property color disabledBg: "#181c21"
    readonly property color disabledBorder: "#262b32"

    // ========================================
    // BORDER RADIUS
    // ========================================
    readonly property int radiusSmall: 6
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 10
    readonly property int radiusXLarge: 12

    // ========================================
    // SPACING
    // ========================================
    readonly property int spacingXSmall: 4
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 16
    readonly property int spacingXLarge: 20
    readonly property int spacingXXLarge: 24

    // ========================================
    // TYPOGRAPHY
    // ========================================
    readonly property int fontXSmall: 10
    readonly property int fontSmall: 12
    readonly property int fontMedium: 14
    readonly property int fontLarge: 16
    readonly property int fontXLarge: 18
    readonly property int fontXXLarge: 20

    readonly property string fontFamily: "Inter, -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
    readonly property string fontFamilyMono: "JetBrains Mono, Consolas, Monaco, 'Courier New', monospace"

    // ========================================
    // OPACITY
    // ========================================
    readonly property real overlayOpacity: 0.95
    readonly property real connectedOpacity: 1.0
    readonly property real disconnectedOpacity: 0.7
    readonly property real hoverOpacity: 0.9

    // ========================================
    // ANIMATION
    // ========================================
    readonly property int colorAnimationDuration: 300
    readonly property int quickAnimationDuration: 150
    readonly property int normalAnimationDuration: 250
    readonly property int slowAnimationDuration: 400

    // ========================================
    // ELEVATION / SHADOWS (simulated via border)
    // ========================================
    readonly property color shadowColor: "#0d0d0d"
    readonly property int shadowWidth: 1
}
