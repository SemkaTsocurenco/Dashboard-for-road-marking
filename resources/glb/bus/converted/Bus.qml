import QtQuick
import QtQuick3D

Node {
    // Materials
    PrincipledMaterial {
        id: main_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus427.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus428.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus428.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus429.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus428.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Blend
    }
    PrincipledMaterial {
        id: roof_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus430.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus431.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus431.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus432.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus431.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: int_3_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus415.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus416.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus416.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus417.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus416.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: wheel_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus439.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus440.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus440.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus441.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus440.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: int_1_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus49.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus410.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus410.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus411.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus410.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: seat_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus433.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus434.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus434.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus435.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus434.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: cpit_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus43.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus44.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus44.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus45.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus44.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: int_2_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus412.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus413.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus413.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus414.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus413.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: kamaz6282_original_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus418.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus419.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus419.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus420.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus419.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: kamaz_salon_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus421.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus422.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus422.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus423.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus422.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: decals_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus46.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus47.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus47.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus48.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus47.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: lights_original_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus424.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus425.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus425.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus426.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus425.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: white_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus442.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus443.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus443.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus444.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus443.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: circles_mat_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus40.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus41.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus41.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus42.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus41.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: wheel_disc_rear_material
        baseColorMap: Texture {
            source: "qrc:/models/bus/maps/bus436.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "qrc:/models/bus/maps/bus437.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "qrc:/models/bus/maps/bus437.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        roughnessChannel: Material.G
        metalness: 1
        roughness: 1
        normalMap: Texture {
            source: "qrc:/models/bus/maps/bus438.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionMap: Texture {
            source: "qrc:/models/bus/maps/bus437.png"
            generateMipmaps: true
            mipFilter: Texture.Nearest
        }
        occlusionChannel: Material.R
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: _material
        metalness: 1
        roughness: 1
        alphaMode: PrincipledMaterial.Opaque
    }
    // end of Materials

    Node {
        id: root
        x: 0.0343934
        y: 1.64484
        z: 0.0407186
        scale.x: 6.05686
        scale.y: 6.05686
        scale.z: 6.05686
        Node {
            id: main_glass
            Model {
                id: main_glass_1
                source: "qrc:/models/bus/meshes/main_glass.mesh"
                materials: [
                    main_mat_material
                ]
            }
        }
        Node {
            id: roof
            Model {
                id: roof_1
                source: "qrc:/models/bus/meshes/roof.mesh"
                materials: [
                    roof_mat_material
                ]
            }
        }
        Node {
            id: int_3
            Model {
                id: int_3_1
                source: "qrc:/models/bus/meshes/int_3.mesh"
                materials: [
                    int_3_mat_material
                ]
            }
        }
        Node {
            id: wheel_02
            Model {
                id: wheel_02_1
                source: "qrc:/models/bus/meshes/wheel_02.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: wheel_rear_tire_1
            Model {
                id: wheel_rear_tire_1_1
                source: "qrc:/models/bus/meshes/wheel_rear_tire_1.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: wheel_00
            Model {
                id: wheel_00_1
                source: "qrc:/models/bus/meshes/wheel_00.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: int_1
            Model {
                id: int_1_1
                source: "qrc:/models/bus/meshes/int_1.mesh"
                materials: [
                    int_1_mat_material
                ]
            }
        }
        Node {
            id: seat_00
            Model {
                id: seat_00_1
                source: "qrc:/models/bus/meshes/seat_00.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_01
            Model {
                id: seat_01_1
                source: "qrc:/models/bus/meshes/seat_01.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: cpit
            Model {
                id: cpit_1
                source: "qrc:/models/bus/meshes/cpit.mesh"
                materials: [
                    cpit_mat_material
                ]
            }
        }
        Node {
            id: seat_02
            Model {
                id: seat_02_1
                source: "qrc:/models/bus/meshes/seat_02.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_03
            Model {
                id: seat_03_1
                source: "qrc:/models/bus/meshes/seat_03.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_04
            Model {
                id: seat_04_1
                source: "qrc:/models/bus/meshes/seat_04.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_05
            Model {
                id: seat_05_1
                source: "qrc:/models/bus/meshes/seat_05.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_06
            Model {
                id: seat_06_1
                source: "qrc:/models/bus/meshes/seat_06.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_07
            Model {
                id: seat_07_1
                source: "qrc:/models/bus/meshes/seat_07.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_08
            Model {
                id: seat_08_1
                source: "qrc:/models/bus/meshes/seat_08.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_09
            Model {
                id: seat_09_1
                source: "qrc:/models/bus/meshes/seat_09.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_10
            Model {
                id: seat_10_1
                source: "qrc:/models/bus/meshes/seat_10.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_11
            Model {
                id: seat_11_1
                source: "qrc:/models/bus/meshes/seat_11.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_12
            Model {
                id: seat_12_1
                source: "qrc:/models/bus/meshes/seat_12.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_13
            Model {
                id: seat_13_1
                source: "qrc:/models/bus/meshes/seat_13.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_14
            Model {
                id: seat_14_1
                source: "qrc:/models/bus/meshes/seat_14.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_15
            Model {
                id: seat_15_1
                source: "qrc:/models/bus/meshes/seat_15.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_16
            Model {
                id: seat_16_1
                source: "qrc:/models/bus/meshes/seat_16.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_17
            Model {
                id: seat_17_1
                source: "qrc:/models/bus/meshes/seat_17.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_18
            Model {
                id: seat_18_1
                source: "qrc:/models/bus/meshes/seat_18.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_19
            Model {
                id: seat_19_1
                source: "qrc:/models/bus/meshes/seat_19.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_20
            Model {
                id: seat_20_1
                source: "qrc:/models/bus/meshes/seat_20.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_21
            Model {
                id: seat_21_1
                source: "qrc:/models/bus/meshes/seat_21.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_22
            Model {
                id: seat_22_1
                source: "qrc:/models/bus/meshes/seat_22.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_23
            Model {
                id: seat_23_1
                source: "qrc:/models/bus/meshes/seat_23.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_24
            Model {
                id: seat_24_1
                source: "qrc:/models/bus/meshes/seat_24.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_25
            Model {
                id: seat_25_1
                source: "qrc:/models/bus/meshes/seat_25.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_26
            Model {
                id: seat_26_1
                source: "qrc:/models/bus/meshes/seat_26.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_27
            Model {
                id: seat_27_1
                source: "qrc:/models/bus/meshes/seat_27.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_28
            Model {
                id: seat_28_1
                source: "qrc:/models/bus/meshes/seat_28.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_29
            Model {
                id: seat_29_1
                source: "qrc:/models/bus/meshes/seat_29.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_30
            Model {
                id: seat_30_1
                source: "qrc:/models/bus/meshes/seat_30.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_31
            Model {
                id: seat_31_1
                source: "qrc:/models/bus/meshes/seat_31.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: seat_32
            Model {
                id: seat_32_1
                source: "qrc:/models/bus/meshes/seat_32.mesh"
                materials: [
                    seat_mat_material
                ]
            }
        }
        Node {
            id: int_2
            Model {
                id: int_2_1
                source: "qrc:/models/bus/meshes/int_2.mesh"
                materials: [
                    int_2_mat_material
                ]
            }
        }
        Node {
            id: body_unchanged
            Model {
                id: body_unchanged_1
                source: "qrc:/models/bus/meshes/body_unchanged.mesh"
                materials: [
                    kamaz6282_original_material
                ]
            }
        }
        Node {
            id: details
            Model {
                id: details_1
                source: "qrc:/models/bus/meshes/details.mesh"
                materials: [
                    kamaz_salon_material
                ]
            }
        }
        Node {
            id: details_2
            Model {
                id: details_3
                source: "qrc:/models/bus/meshes/details_1.mesh"
                materials: [
                    decals_material
                ]
            }
        }
        Node {
            id: details_4
            Model {
                id: details_5
                source: "qrc:/models/bus/meshes/details_2.mesh"
                materials: [
                    lights_original_material
                ]
            }
        }
        Node {
            id: wheel_rear_tire_002
            Model {
                id: wheel_rear_tire_002_1
                source: "qrc:/models/bus/meshes/wheel_rear_tire_002.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: wheel_rear_tire_003
            Model {
                id: wheel_rear_tire_003_1
                source: "qrc:/models/bus/meshes/wheel_rear_tire_003.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: wheel_rear_tire_004
            Model {
                id: wheel_rear_tire_004_1
                source: "qrc:/models/bus/meshes/wheel_rear_tire_004.mesh"
                materials: [
                    wheel_mat_material
                ]
            }
        }
        Node {
            id: node430107_fr
            Model {
                id: node430107_fr_1
                source: "qrc:/models/bus/meshes/node430107_fr.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: node430107_re
            Model {
                id: node430107_re_1
                source: "qrc:/models/bus/meshes/node430107_re.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: eto_electrobus_L
            Model {
                id: eto_electrobus_L_1
                source: "qrc:/models/bus/meshes/eto_electrobus_L.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: side_text_1
            Model {
                id: side_text_1_1
                source: "qrc:/models/bus/meshes/side_text_1.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: side_text_2
            Model {
                id: side_text_2_1
                source: "qrc:/models/bus/meshes/side_text_2.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: front_logo
            Model {
                id: front_logo_1
                source: "qrc:/models/bus/meshes/front_logo.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: eto_electrobus_Back
            Model {
                id: eto_electrobus_Back_1
                source: "qrc:/models/bus/meshes/eto_electrobus_Back.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: kamaz
            Model {
                id: kamaz_1
                source: "qrc:/models/bus/meshes/kamaz.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: eto_electrobus_R
            Model {
                id: eto_electrobus_R_1
                source: "qrc:/models/bus/meshes/eto_electrobus_R.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: electrobus_moskva
            Model {
                id: electrobus_moskva_1
                source: "qrc:/models/bus/meshes/electrobus_moskva.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: circles_back
            Model {
                id: circles_back_1
                source: "qrc:/models/bus/meshes/circles_back.mesh"
                materials: [
                    circles_mat_material
                ]
            }
        }
        Node {
            id: circles_fr
            Model {
                id: circles_fr_1
                source: "qrc:/models/bus/meshes/circles_fr.mesh"
                materials: [
                    circles_mat_material
                ]
            }
        }
        Node {
            id: circles_R
            Model {
                id: circles_R_1
                source: "qrc:/models/bus/meshes/circles_R.mesh"
                materials: [
                    circles_mat_material
                ]
            }
        }
        Node {
            id: mosgortrans_logo_on_plane
            Model {
                id: mosgortrans_logo_on_plane_1
                source: "qrc:/models/bus/meshes/mosgortrans_logo_on_plane.mesh"
                materials: [
                    circles_mat_material
                ]
            }
        }
        Node {
            id: plane001_pod_logo
            Model {
                id: plane001_pod_logo_1
                source: "qrc:/models/bus/meshes/plane001_pod_logo.mesh"
                materials: [
                    white_mat_material
                ]
            }
        }
        Node {
            id: wheel_rear_disc_1
            Model {
                id: wheel_rear_disc_1_1
                source: "qrc:/models/bus/meshes/wheel_rear_disc_1.mesh"
                materials: [
                    wheel_disc_rear_material
                ]
            }
        }
        Node {
            id: wheel_rear_disc_002
            Model {
                id: wheel_rear_disc_002_1
                source: "qrc:/models/bus/meshes/wheel_rear_disc_002.mesh"
                materials: [
                    wheel_disc_rear_material
                ]
            }
        }
    }
}
