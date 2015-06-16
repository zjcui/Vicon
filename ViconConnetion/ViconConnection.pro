CONFIG  +=  qt \
            console \
            static
QT 	  +=	core \
		sql
HEADERS +=  sqlhandler.h \
		viconrecorder.h \
    Vicon/math/External/gmtl_OpenSGConvert.h \
    Vicon/math/Util/gmtl_Assert.h \
    Vicon/math/Util/gmtl_Meta.h \
    Vicon/math/Util/gmtl_StaticAssert.h \
    Vicon/math/gmtl_AABox.h \
    Vicon/math/gmtl_AABoxOps.h \
    Vicon/math/gmtl_AxisAngle.h \
    Vicon/math/gmtl_AxisAngleOps.h \
    Vicon/math/gmtl_Comparitors.h \
    Vicon/math/gmtl_Containment.h \
    Vicon/math/gmtl_Coord.h \
    Vicon/math/gmtl_CoordOps.h \
    Vicon/math/gmtl_Defines.h \
    Vicon/math/gmtl_EulerAngle.h \
    Vicon/math/gmtl_EulerAngleOps.h \
    Vicon/math/gmtl_Generate.h \
    Vicon/math/gmtl_Intersection.h \
    Vicon/math/gmtl_LineSeg.h \
    Vicon/math/gmtl_LineSegOps.h \
    Vicon/math/gmtl_Math.h \
    Vicon/math/gmtl_Matrix.h \
    Vicon/math/gmtl_MatrixOps.h \
    Vicon/math/gmtl_Output.h \
    Vicon/math/gmtl_Plane.h \
    Vicon/math/gmtl_PlaneOps.h \
    Vicon/math/gmtl_Point.h \
    Vicon/math/gmtl_PointOps.h \
    Vicon/math/gmtl_Quat.h \
    Vicon/math/gmtl_QuatOps.h \
    Vicon/math/gmtl_Ray.h \
    Vicon/math/gmtl_Sphere.h \
    Vicon/math/gmtl_SphereOps.h \
    Vicon/math/gmtl_Tri.h \
    Vicon/math/gmtl_TriOps.h \
    Vicon/math/gmtl_Vec.h \
    Vicon/math/gmtl_VecBase.h \
    Vicon/math/gmtl_VecOps.h \
    Vicon/math/gmtl_Version.h \
    Vicon/math/gmtl_Xforms.h \
    Vicon/math/j_Frustum.h \
    Vicon/math/j_Math.h \
    Vicon/math/j_MathFast.h \
    Vicon/math/j_MathFunc.h \
    Vicon/math/j_MathTypes.h \
    Vicon/math/j_TMComposer.h \
    Vicon/utility/j_Bucket.h \
    Vicon/utility/j_factory.h \
    Vicon/utility/j_Manager.h \
    Vicon/utility/j_RadixSort.h \
    Vicon/utility/j_Registry.h \
    Vicon/utility/j_SimpleFactory.h \
    Vicon/utility/j_SimpleSorts.h \
    Vicon/utility/j_Utility.h \
    Vicon/j_Mocap.h \
    Vicon/j_MocapCodes.h \
    viconbody.h
SOURCES +=  main.cpp \
		sqlhandler.cpp \
		viconrecorder.cpp \
    Vicon/math/j_TMComposer.cpp \
    Vicon/utility/j_RadixSort.cpp \
    Vicon/utility/j_SimpleSorts.cpp \
    Vicon/utility/j_Utility.cpp \
    Vicon/j_Mocap.cpp \
    viconbody.cpp
INCLUDEPATH += $$PWD/Vicon \
                $$PWD/Vicon/math \
                $$PWD/Vicon/utility
LIBS += -lws2_32 -lwsock32
