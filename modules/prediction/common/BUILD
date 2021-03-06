load("//tools:cpplint.bzl", "cpplint")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "prediction_gflags",
    srcs = [
        "prediction_gflags.cc",
        "prediction_system_gflags.cc",
    ],
    hdrs = [
        "prediction_gflags.h",
        "prediction_system_gflags.h",
    ],
    deps = [
        "//external:gflags",
    ],
)

cc_library(
    name = "prediction_util",
    srcs = ["prediction_util.cc"],
    hdrs = ["prediction_util.h"],
    deps = [
        ":prediction_gflags",
        ":prediction_map",
        "//cyber",
        "//modules/common/math",
        "//modules/common/proto:pnc_point_proto",
        "//modules/prediction/proto:lane_graph_proto",
        "@eigen",
    ],
)

cc_test(
    name = "prediction_util_test",
    size = "small",
    srcs = ["prediction_util_test.cc"],
    deps = [
        ":prediction_util",
        "@gtest//:main",
    ],
)

cc_library(
    name = "prediction_map",
    srcs = ["prediction_map.cc"],
    hdrs = ["prediction_map.h"],
    deps = [
        ":prediction_gflags",
        "//modules/common/math:geometry",
        "//modules/common/math:linear_interpolation",
        "//modules/map/hdmap:hdmap_util",
        "//modules/map/pnc_map",
        "@eigen",
    ],
)

cc_test(
    name = "prediction_map_test",
    size = "small",
    srcs = ["prediction_map_test.cc"],
    data = [
        "//modules/prediction:prediction_data",
        "//modules/prediction:prediction_testdata",
    ],
    deps = [
        ":kml_map_based_test",
        ":prediction_gflags",
        ":prediction_map",
        "@gtest//:main",
    ],
)

cc_library(
    name = "feature_output",
    srcs = ["feature_output.cc"],
    hdrs = ["feature_output.h"],
    deps = [
        "//cyber",
        "//modules/common/util",
        "//modules/prediction/common:prediction_gflags",
        "//modules/prediction/proto:feature_proto",
        "//modules/prediction/proto:offline_features_proto",
    ],
)

cc_test(
    name = "feature_output_test",
    size = "small",
    srcs = ["feature_output_test.cc"],
    deps = [
        ":feature_output",
        "@gtest//:main",
    ],
)

cc_library(
    name = "road_graph",
    srcs = ["road_graph.cc"],
    hdrs = ["road_graph.h"],
    deps = [
        ":prediction_map",
        "//modules/common/status",
        "//modules/map/hdmap",
        "//modules/prediction/proto:lane_graph_proto",
    ],
)

cc_test(
    name = "road_graph_test",
    size = "small",
    srcs = ["road_graph_test.cc"],
    data = [
        "//modules/prediction:prediction_data",
        "//modules/prediction:prediction_testdata",
    ],
    deps = [
        ":kml_map_based_test",
        ":prediction_gflags",
        ":prediction_map",
        ":road_graph",
        "@gtest//:main",
    ],
)

cc_library(
    name = "kml_map_based_test",
    hdrs = ["kml_map_based_test.h"],
    deps = [
        "//modules/common/configs:config_gflags",
        "@gtest",
    ],
)

cc_library(
    name = "validation_checker",
    srcs = ["validation_checker.cc"],
    hdrs = ["validation_checker.h"],
    deps = [
        "//modules/common/math",
        "//modules/common/proto:pnc_point_proto",
        "//modules/prediction/common:prediction_gflags",
        "//modules/prediction/proto:lane_graph_proto",
    ],
)

cc_test(
    name = "validation_checker_test",
    size = "small",
    srcs = ["validation_checker_test.cc"],
    data = [
        "//modules/prediction:prediction_data",
        "//modules/prediction:prediction_testdata",
    ],
    deps = [
        ":validation_checker",
        "@gtest//:main",
    ],
)

cc_library(
    name = "environment_features",
    srcs = ["environment_features.cc"],
    hdrs = ["environment_features.h"],
    deps = [
        "//cyber",
        "//modules/common/proto:geometry_proto",
    ],
)

cc_test(
    name = "environment_features_test",
    size = "small",
    srcs = ["environment_features_test.cc"],
    deps = [
        ":environment_features",
        "//cyber",
        "//modules/common/proto:geometry_proto",
        "@gtest//:main",
    ],
)

cc_library(
    name = "junction_analyzer",
    srcs = ["junction_analyzer.cc"],
    hdrs = ["junction_analyzer.h"],
    deps = [
        ":prediction_map",
        "//cyber",
        "//modules/map/hdmap",
        "//modules/prediction/proto:feature_proto",
    ],
)

cc_test(
    name = "junction_analyzer_test",
    size = "small",
    srcs = ["junction_analyzer_test.cc"],
    data = [
        "//modules/prediction:prediction_data",
        "//modules/prediction:prediction_testdata",
    ],
    deps = [
        ":junction_analyzer",
        ":kml_map_based_test",
        ":prediction_gflags",
        ":prediction_map",
        "@gtest//:main",
    ],
)

cpplint()
