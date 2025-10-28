from androguard.misc import AnalyzeAPK

apk_path = r"e:/dashboard/decoder_v1.2_20250903.apk"

a, d, dx = AnalyzeAPK(apk_path)
print("Package:", a.get_package())
print("Activities:")
for act in a.get_activities():
    print("  ", act)
print("Libraries:", a.get_libraries())
print("Permissions:")
for perm in a.get_permissions():
    print("  ", perm)
print("Receivers:")
for recv in a.get_receivers():
    print("  ", recv)
print("Providers:")
for prov in a.get_providers():
    print("  ", prov)

interesting = []
interesting_methods = {}
for string_analysis in dx.get_strings():
    value = string_analysis.get_value()
    if not isinstance(value, str):
        continue
    lowered = value.lower()
    if not any(key in lowered for key in ("rtsp", "rtmp", "webrtc", "hevc", "h265", "mediacodec", "ffmpeg", "gst")):
        continue
    for ref in string_analysis.get_xref_from():
        if not isinstance(ref, tuple) or len(ref) < 2:
            continue

        ref_class = None
        ref_method = None

        if len(ref) == 3:
            ref_class, ref_method, _ = ref
        else:
            ref_class, ref_method = ref

        # Determine class name
        class_name = None
        if ref_class is not None:
            if hasattr(ref_class, "get_name"):
                class_name = ref_class.get_name()
            elif hasattr(ref_class, "get_class_name"):
                class_name = ref_class.get_class_name()

        # Resolve method definition
        method_def = None
        if ref_method is not None:
            if hasattr(ref_method, "get_method"):
                method_def = ref_method.get_method()
            elif hasattr(ref_method, "get_descriptor"):
                method_def = ref_method

        if method_def is None:
            continue

        if class_name is None and hasattr(method_def, "get_class_name"):
            class_name = method_def.get_class_name()

        interesting.append((class_name or "<unknown>", method_def.get_name(), method_def.get_descriptor(), value))
        key = (method_def.get_class_name(), method_def.get_name(), method_def.get_descriptor())
        interesting_methods.setdefault(key, method_def)

print("\nInteresting string hits:")
for cls, name, desc, value in interesting:
    print(f"[{cls} -> {name}{desc}] {value}")

if interesting_methods:
    print("\nDisassembly of relevant methods:")
    for (cls, name, desc), method in interesting_methods.items():
        print(f"\n== {cls}->{name}{desc} ==")
        code = method.get_code()
        if code is None:
            print("  <no code>")
            continue
        for ins in code.get_bc().get_instructions():
            print("  ", ins.get_name(), ins.get_output())
