/*************************************************************************/
/*  openxr_android_extension.cpp                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "openxr_android_extension.h"
#include "java_godot_wrapper.h"
#include "os_android.h"
#include "thread_jandroid.h"

#include <jni.h>
#include <modules/openxr/openxr_api.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

OpenXRAndroidExtension *OpenXRAndroidExtension::singleton = nullptr;

OpenXRAndroidExtension *OpenXRAndroidExtension::get_singleton() {
	return singleton;
}

OpenXRAndroidExtension::OpenXRAndroidExtension(OpenXRAPI *p_openxr_api) :
		OpenXRExtensionWrapper(p_openxr_api) {
	singleton = this;
	request_extensions[XR_KHR_LOADER_INIT_ANDROID_EXTENSION_NAME] = nullptr; // must be available
	request_extensions[XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME] = &create_instance_extension_available;
}

void OpenXRAndroidExtension::on_before_instance_created() {
	EXT_INIT_XR_FUNC(xrInitializeLoaderKHR);

	JNIEnv *env = get_jni_env();
	JavaVM *vm;
	env->GetJavaVM(&vm);
	jobject activity_object = env->NewGlobalRef(static_cast<OS_Android *>(OS::get_singleton())->get_godot_java()->get_activity());

	XrLoaderInitInfoAndroidKHR loader_init_info_android = {
		.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR,
		.next = nullptr,
		.applicationVM = vm,
		.applicationContext = activity_object
	};
	XrResult result = xrInitializeLoaderKHR((const XrLoaderInitInfoBaseHeaderKHR *)&loader_init_info_android);
	ERR_FAIL_COND_MSG(XR_FAILED(result), "Failed to call xrInitializeLoaderKHR");
}

// We're keeping the Android create info struct here to avoid including openxr_platform.h in a header, which would break other extensions.
// This is reasonably safe as the struct is only used during intialization and the extension is a singleton.
static XrInstanceCreateInfoAndroidKHR instance_create_info;

void *OpenXRAndroidExtension::set_instance_create_info_and_get_next_pointer(void *p_next_pointer) {
	if (!create_instance_extension_available) {
		return nullptr;
	}

	JNIEnv *env = get_jni_env();
	JavaVM *vm;
	env->GetJavaVM(&vm);
	jobject activity_object = env->NewGlobalRef(static_cast<OS_Android *>(OS::get_singleton())->get_godot_java()->get_activity());

	instance_create_info = {
		.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR,
		.next = p_next_pointer,
		.applicationVM = vm,
		.applicationActivity = activity_object
	};
	return &instance_create_info;
}

OpenXRAndroidExtension::~OpenXRAndroidExtension() {
	singleton = nullptr;
}
