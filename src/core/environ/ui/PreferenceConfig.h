static tPreferenceScreen RootPreference;
static tPreferenceScreen* OpenglOptPreference, *SoftRendererOptPreference;
static Size PrefListSize;

class tTVPPreferenceInfoCheckBox : public tTVPPreferenceInfo<bool> {
public:
	tTVPPreferenceInfoCheckBox(const std::string &cap, const std::string &key, bool defval)
		: tTVPPreferenceInfo<bool>(cap, key, defval) {}
	virtual iPreferenceItem *createItem() override {
		GlobalConfigManager *mgr = GlobalConfigManager::GetInstance();
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		return CreatePreferenceItem<tPreferenceItemCheckBox>(PrefListSize, locmgr->GetText(Caption),
			[mgr, this](tPreferenceItemCheckBox* item) {
			item->_getter = std::bind(&GlobalConfigManager::GetValueBool, mgr, Key, DefaultValue);
			item->_setter = [this](bool v){
				GlobalConfigManager::GetInstance()->SetValueInt(Key, v);
			};
		});
	}
};

class tTVPPreferenceInfoSelectList : public tTVPPreferenceInfo<std::string>, tPreferenceItemSelectListInfo {
public:
	tTVPPreferenceInfoSelectList(const std::string &cap, const std::string &key, const std::string &defval,
		const std::initializer_list<std::pair<std::string, std::string> > &listinfo)
		: tTVPPreferenceInfo<std::string>(cap, key, defval), ListInfo(listinfo){}
	virtual iPreferenceItem *createItem() override {
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		return CreatePreferenceItem<tPreferenceItemSelectList>(PrefListSize, locmgr->GetText(Caption),
			[this](tPreferenceItemSelectList* item) {
			item->initInfo(this);
			item->_getter = std::bind(&GlobalConfigManager::GetValueString, GlobalConfigManager::GetInstance(), Key, DefaultValue);
			item->_setter = [this](std::string v){
				GlobalConfigManager::GetInstance()->SetValue(Key, v);
			};
		});
	}
	virtual const std::vector<std::pair<std::string, std::string> >& getListInfo() const override {
		return ListInfo;
	}
	std::vector<std::pair<std::string, std::string> > ListInfo;
};

class tTVPPreferenceInfoSelectFile : public tTVPPreferenceInfo<std::string> {
public:
	tTVPPreferenceInfoSelectFile(const std::string &cap, const std::string &key, const std::string &defval)
		: tTVPPreferenceInfo<std::string>(cap, key, defval) {}

	virtual iPreferenceItem *createItem() override {
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		return CreatePreferenceItem<tPreferenceItemFileSelect>(PrefListSize, locmgr->GetText(Caption),
			[this](tPreferenceItemFileSelect* item) {
			item->_getter = std::bind(&GlobalConfigManager::GetValueString, GlobalConfigManager::GetInstance(), Key, DefaultValue);
			item->_setter = [this](std::string v) {
				GlobalConfigManager::GetInstance()->SetValue(Key, v);
			};
		});
	}
};

class tTVPPreferenceInfoSubPref : public iTVPPreferenceInfo {
public:
	tTVPPreferenceInfoSubPref(const std::string &cap) { Caption = cap; } // Key is useless
	virtual iPreferenceItem *createItem() override {
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		iPreferenceItem *ret = CreatePreferenceItem<tPreferenceItemSubDir>(PrefListSize, locmgr->GetText(Caption));
		ret->addClickEventListener([](Ref*){
			std::string renderer = GlobalConfigManager::GetInstance()->GetValueString("renderer");
			if (renderer == "opengl")
				TVPMainScene::GetInstance()->pushUIForm(TVPGlobalPreferenceForm::create(OpenglOptPreference));
			else if (renderer == "software")
				TVPMainScene::GetInstance()->pushUIForm(TVPGlobalPreferenceForm::create(SoftRendererOptPreference));
		});
		return ret;
	}
};

class tTVPPreferenceInfoSliderIcon : public tTVPPreferenceInfo<float> {
public:
	tTVPPreferenceInfoSliderIcon(const std::string &cap, const std::string &key, float defval)
		: tTVPPreferenceInfo<float>(cap, key, defval) {}
	virtual iPreferenceItem *createItem() override {
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		GlobalConfigManager *mgr = GlobalConfigManager::GetInstance();

		tPreferenceItemCursorSlider * ret = new tPreferenceItemCursorSlider(DefaultValue, TVPMainScene::convertCursorScale);
		ret->autorelease();
		ret->_getter = std::bind(&GlobalConfigManager::GetValueFloat, mgr, Key, DefaultValue);
		ret->_setter = std::bind(&GlobalConfigManager::SetValueFloat, mgr, Key, std::placeholders::_1);
		ret->initFromInfo(PrefListSize, locmgr->GetText(Caption));
		return ret;
	}
};

class tTVPPreferenceInfoSliderText : public tTVPPreferenceInfo<float> {
public:
	tTVPPreferenceInfoSliderText(const std::string &cap, const std::string &key, float defval)
		: tTVPPreferenceInfo<float>(cap, key, defval) {}

	static std::string convertPercentScale(float scale) {
		char buf[16];
		sprintf(buf, "%d%%", (int)(scale * 100));
		return buf;
	}

	virtual iPreferenceItem *createItem() override {
		LocaleConfigManager *locmgr = LocaleConfigManager::GetInstance();
		GlobalConfigManager *mgr = GlobalConfigManager::GetInstance();

		tPreferenceItemTextSlider * ret = new tPreferenceItemTextSlider(DefaultValue, convertPercentScale);
		ret->autorelease();
		ret->_getter = std::bind(&GlobalConfigManager::GetValueFloat, mgr, Key, DefaultValue);
		ret->_setter = std::bind(&GlobalConfigManager::SetValueFloat, mgr, Key, std::placeholders::_1);
		ret->initFromInfo(PrefListSize, locmgr->GetText(Caption));
		return ret;
	}
};

static void initAllConfig() {
	if (!RootPreference.Preferences.empty()) return;
	RootPreference.Title = "preference_title";
	RootPreference.Preferences = {
		new tTVPPreferenceInfoCheckBox("preference_output_log", "outputlog", true),
		new tTVPPreferenceInfoCheckBox("preference_show_fps", "showfps", false),
		new tTVPPreferenceInfoSelectList("preference_select_renderer", "renderer", "software", {
			{ "preference_opengl", "opengl" },
			{ "preference_software", "software" }
		}),
		new tTVPPreferenceInfoSubPref("preference_renderer_opt"),
		new tTVPPreferenceInfoSelectFile("preference_default_font", "default_font", ""),
		new tTVPPreferenceInfoSelectList("preference_mem_limit", "memusage", "unlimited", {
			{ "preference_mem_unlimited", "unlimited" },
			{ "preference_mem_high", "high" },
			{ "preference_mem_medium", "medium" },
			{ "preference_mem_low", "low" }
			}),
			new tTVPPreferenceInfoCheckBox("preference_keep_screen_alive", "keep_screen_alive", true),
			new tTVPPreferenceInfoSliderIcon("preference_virtual_cursor_scale", "vcursor_scale", 0.5f),
			new tTVPPreferenceInfoSliderText("preference_menu_handler_opacity", "menu_handler_opa", 0.15f),
#ifdef GLOBAL_PREFERENCE
			new tTVPPreferenceInfoCheckBox("preference_remember_last_path", "remember_last_path", true),
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
			new tTVPPreferenceInfoCheckBox("preference_hide_android_sys_btn", "hide_android_sys_btn", false),
#endif
#endif
#if 0
			new tTVPPreferenceInfo(tTVPPreferenceInfo::eTypeSubPref, "preference_custom_option"),
#endif
			nullptr
	};

	SoftRendererOptPreference = new tPreferenceScreen("preference_soft_renderer_opt", {
		new tTVPPreferenceInfoSelectList("preference_multi_draw_thread", "software_draw_thread", "0", {
			{ "preference_draw_thread_auto", "0" },
			{ "preference_draw_thread_1", "1" },
			{ "preference_draw_thread_2", "2" },
			{ "preference_draw_thread_3", "3" },
			{ "preference_draw_thread_4", "4" },
			{ "preference_draw_thread_5", "5" },
			{ "preference_draw_thread_6", "6" },
			{ "preference_draw_thread_7", "7" },
			{ "preference_draw_thread_8", "8" }
		}),
		new tTVPPreferenceInfoSelectList("preference_software_compress_tex", "software_compress_tex", "none", {
			{ "preference_soft_compress_tex_none", "none" },
			{ "preference_soft_compress_tex_halfline", "halfline" },
// 			{ "preference_soft_compress_tex_quarter", "quarter" },
// 			{ "preference_soft_compress_tex_lz4", "lz4" }
		}),
	});

	OpenglOptPreference = new tPreferenceScreen("preference_opengl_renderer_opt", {
		new tTVPPreferenceInfoCheckBox("preference_ogl_accurate_render", "ogl_accurate_render", false),
		new tTVPPreferenceInfoCheckBox("preference_opengl_dup_target", "ogl_dup_target", true),
		new tTVPPreferenceInfoSelectList("preference_ogl_max_texsize", "ogl_max_texsize", "0", {
			{ "preference_ogl_texsize_auto", "0" },
			{ "preference_ogl_texsize_1024", "1024" },
			{ "preference_ogl_texsize_2048", "2048" },
			{ "preference_ogl_texsize_4096", "4096" },
			{ "preference_ogl_texsize_8192", "8192" },
			{ "preference_ogl_texsize_16384", "16384" }
		}),
		new tTVPPreferenceInfoSelectList("preference_ogl_compress_tex", "ogl_compress_tex", "none", {
			{ "preference_ogl_compress_tex_none", "none" },
			{ "preference_ogl_compress_tex_half", "half" },
		}),
// 		new tTVPPreferenceInfoSelectList("preference_ogl_render_tex_quality", "ogl_render_tex_quality", "1", {
// 			{ "preference_ogl_render_tex_quality_100", "1" },
// 			{ "preference_ogl_render_tex_quality_75", "0.75" },
// 			{ "preference_ogl_render_tex_quality_50", "0.5" }
// 		}),
	});
}