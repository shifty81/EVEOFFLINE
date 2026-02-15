add_executable(AtlasEditor
    main.cpp
    ui/EditorLayout.cpp
    panels/ECSInspectorPanel.cpp
    panels/NetInspectorPanel.cpp
    panels/ConsolePanel.cpp
    tools/GamePackagerPanel.cpp
)

target_link_libraries(AtlasEditor AtlasEngine)
