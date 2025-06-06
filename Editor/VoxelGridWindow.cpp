#include "stdafx.h"
#include "VoxelGridWindow.h"

using namespace wi::ecs;
using namespace wi::scene;

void VoxelGridWindow::Create(EditorComponent* _editor)
{
	editor = _editor;
	wi::gui::Window::Create(ICON_VOXELGRID " VoxelGrid", wi::gui::Window::WindowControls::COLLAPSE | wi::gui::Window::WindowControls::CLOSE | wi::gui::Window::WindowControls::FIT_ALL_WIDGETS_VERTICAL);
	SetSize(XMFLOAT2(520, 480));

	closeButton.SetTooltip("Delete VoxelGrid");
	OnClose([=](wi::gui::EventArgs args) {

		wi::Archive& archive = editor->AdvanceHistory();
		archive << EditorComponent::HISTORYOP_COMPONENT_DATA;
		editor->RecordEntity(archive, entity);

		editor->GetCurrentScene().voxel_grids.Remove(entity);

		editor->RecordEntity(archive, entity);

		editor->componentsWnd.RefreshEntityTree();
		});

	infoLabel.Create("");
	infoLabel.SetFitTextEnabled(true);
	AddWidget(&infoLabel);

	dimXInput.Create("DimX");
	dimXInput.SetDescription("Dimension X: ");
	dimXInput.OnInputAccepted([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		voxelgrid->init(uint32_t(std::max(1, args.iValue)), voxelgrid->resolution.y, voxelgrid->resolution.z);
	});
	AddWidget(&dimXInput);

	dimYInput.Create("DimY");
	dimYInput.SetDescription("Y: ");
	dimYInput.OnInputAccepted([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		voxelgrid->init(voxelgrid->resolution.x, uint32_t(std::max(1, args.iValue)), voxelgrid->resolution.z);
		});
	AddWidget(&dimYInput);

	dimZInput.Create("DimZ");
	dimZInput.SetDescription("Z: ");
	dimZInput.OnInputAccepted([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		voxelgrid->init(voxelgrid->resolution.x, voxelgrid->resolution.y, uint32_t(std::max(1, args.iValue)));
		});
	AddWidget(&dimZInput);

	clearButton.Create("Clear voxels " ICON_CLEARVOXELS);
	clearButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		voxelgrid->cleardata();
	});
	AddWidget(&clearButton);

	voxelizeObjectsButton.Create("Voxelize objects " ICON_VOXELIZE);
	voxelizeObjectsButton.SetTooltip("Generate navigation grid including all meshes.");
	voxelizeObjectsButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		scene.VoxelizeScene(*voxelgrid, subtractCheckBox.GetCheck(), wi::enums::FILTER_OBJECT_ALL);
	});
	AddWidget(&voxelizeObjectsButton);

	voxelizeNavigationButton.Create("Voxelize navigation " ICON_VOXELIZE);
	voxelizeNavigationButton.SetTooltip("Generate navigation grid including all navmeshes (object tagged as navmesh).");
	voxelizeNavigationButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		scene.VoxelizeScene(*voxelgrid, subtractCheckBox.GetCheck(), wi::enums::FILTER_NAVIGATION_MESH);
		});
	AddWidget(&voxelizeNavigationButton);

	voxelizeCollidersButton.Create("Voxelize CPU colliders " ICON_VOXELIZE);
	voxelizeCollidersButton.SetTooltip("Generate navigation grid including all CPU colliders.");
	voxelizeCollidersButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		scene.VoxelizeScene(*voxelgrid, subtractCheckBox.GetCheck(), wi::enums::FILTER_COLLIDER);
		});
	AddWidget(&voxelizeCollidersButton);

	floodfillButton.Create("Flood fill " ICON_VOXELFILL);
	floodfillButton.SetTooltip("Fill enclosed empty voxel areas to solid.\nThis can take long if there are large enclosed empty areas.");
	floodfillButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		wi::Timer tim;
		voxelgrid->flood_fill();
		wilog(text, arraysize(text), "Flood filling took %.2f seconds.", tim.elapsed_seconds());
		});
	AddWidget(&floodfillButton);

	fitToSceneButton.Create("Fit bounds to scene " ICON_VOXELBOUNDS);
	fitToSceneButton.SetTooltip("Fit the bounds of the voxel grid onto the whole scene.");
	fitToSceneButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		if (scene.bounds.getArea() < 0)
			return;
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		voxelgrid->from_aabb(scene.bounds);
		TransformComponent* transform = scene.transforms.GetComponent(entity);
		if (transform != nullptr)
		{
			// feed back to transform component if it exists:
			transform->translation_local = voxelgrid->center;
			transform->scale_local = voxelgrid->voxelSize;
			transform->SetDirty();
		}
	});
	AddWidget(&fitToSceneButton);

	generateMeshButton.Create("Generate Mesh " ICON_MESH);
	generateMeshButton.SetTooltip("Generate a mesh from the voxels.");
	generateMeshButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		wi::vector<uint32_t> indices;
		wi::vector<XMFLOAT3> vertices;
		voxelgrid->create_mesh(indices, vertices, false);
		if (vertices.empty())
		{
			wi::backlog::post("VoxelGrid.create_mesh() : no voxels were found, so mesh creation is aborted.", wi::backlog::LogLevel::Warning);
			return;
		}
		scene.Entity_CreateMeshFromData("voxelgrid_to_mesh", indices.size(), indices.data(), vertices.size(), vertices.data());
	});
	AddWidget(&generateMeshButton);

	generateSimplifiedMeshButton.Create("Generate Simplified Mesh " ICON_MESH);
	generateSimplifiedMeshButton.SetTooltip("Generate a simplified mesh from the voxels.");
	generateSimplifiedMeshButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		wi::vector<uint32_t> indices;
		wi::vector<XMFLOAT3> vertices;
		voxelgrid->create_mesh(indices, vertices, true);
		if (vertices.empty())
		{
			wi::backlog::post("VoxelGrid.create_mesh() : no voxels were found, so mesh creation is aborted.", wi::backlog::LogLevel::Warning);
			return;
		}
		scene.Entity_CreateMeshFromData("voxelgrid_to_mesh", indices.size(), indices.data(), vertices.size(), vertices.data());
	});
	AddWidget(&generateSimplifiedMeshButton);

	generateNavMeshButton.Create("Generate Navigation Mesh " ICON_NAVIGATION);
	generateNavMeshButton.SetTooltip("Generate a simplified navigation mesh from the voxels.");
	generateNavMeshButton.OnClick([=](wi::gui::EventArgs args) {
		Scene& scene = editor->GetCurrentScene();
		wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);
		if (voxelgrid == nullptr)
			return;
		wi::vector<uint32_t> indices;
		wi::vector<XMFLOAT3> vertices;
		voxelgrid->create_mesh(indices, vertices, true, &scene);
		if (vertices.empty())
		{
			wi::backlog::post("VoxelGrid.create_mesh() : no voxels were found, so mesh creation is aborted.", wi::backlog::LogLevel::Warning);
			return;
		}
		Entity entity = scene.Entity_CreateMeshFromData("voxelgrid_to_navmesh", indices.size(), indices.data(), vertices.size(), vertices.data());
		ObjectComponent* object = scene.objects.GetComponent(entity);
		if (object != nullptr)
		{
			object->filterMask |= wi::enums::FILTER_NAVIGATION_MESH;

			MeshComponent* mesh = scene.meshes.GetComponent(object->meshID);
			if (mesh != nullptr)
			{
				mesh->SetBVHEnabled(true);
			}
		}
	});
	AddWidget(&generateNavMeshButton);

	subtractCheckBox.Create("Subtraction mode: ");
	subtractCheckBox.SetTooltip("If enabled, voxelization will be subtractive, so it will remove voxels instead of add.");
	AddWidget(&subtractCheckBox);

	debugAllCheckBox.Create("Debug draw all: ");
	debugAllCheckBox.SetTooltip("Draw all voxel grids, whether they are selected or not.");
	AddWidget(&debugAllCheckBox);


	SetMinimized(true);
	SetVisible(false);

}

void VoxelGridWindow::SetEntity(Entity entity)
{
	bool changed = this->entity != entity;
	this->entity = entity;

	Scene& scene = editor->GetCurrentScene();
	wi::VoxelGrid* voxelgrid = scene.voxel_grids.GetComponent(entity);

	if (voxelgrid != nullptr)
	{
		std::string infotext = "Voxel grid can be used for navigation. By voxelizing the scene into the grid, path finding can be used on the resulting voxel grid.";
		infotext += "\n\nMemory usage: ";
		infotext += wi::helper::GetMemorySizeText(voxelgrid->get_memory_size());
		infoLabel.SetText(infotext);

		dimXInput.SetValue((int)voxelgrid->resolution.x);
		dimYInput.SetValue((int)voxelgrid->resolution.y);
		dimZInput.SetValue((int)voxelgrid->resolution.z);
	}
}

void VoxelGridWindow::ResizeLayout()
{
	wi::gui::Window::ResizeLayout();
	layout.margin_left = 90;

	layout.add_fullwidth(infoLabel);

	const float padding2 = 20;
	const float l = 95;
	const float r = layout.width;
	float w = ((r - l) - padding2 * 2) / 3.0f;
	dimXInput.SetSize(XMFLOAT2(w, dimXInput.GetSize().y));
	dimYInput.SetSize(XMFLOAT2(w, dimYInput.GetSize().y));
	dimZInput.SetSize(XMFLOAT2(w, dimZInput.GetSize().y));
	dimXInput.SetPos(XMFLOAT2(layout.margin_left, layout.y));
	dimYInput.SetPos(XMFLOAT2(dimXInput.GetPos().x + w + padding2, layout.y));
	dimZInput.SetPos(XMFLOAT2(dimYInput.GetPos().x + w + padding2, layout.y));
	layout.y += dimZInput.GetSize().y;
	layout.y += layout.padding;

	layout.add_fullwidth(clearButton);
	layout.add_fullwidth(voxelizeObjectsButton);
	layout.add_fullwidth(voxelizeCollidersButton);
	layout.add_fullwidth(voxelizeNavigationButton);
	layout.add_fullwidth(floodfillButton);
	layout.add_fullwidth(fitToSceneButton);
	layout.add_fullwidth(generateMeshButton);
	layout.add_fullwidth(generateSimplifiedMeshButton);
	layout.add_fullwidth(generateNavMeshButton);
	layout.add_right(subtractCheckBox);
	layout.add_right(debugAllCheckBox);
}
