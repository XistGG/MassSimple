#
# UE_FixOutOfDatePoseAssets.py
#
# Copyright (c) 2025 Xist.GG LLC
# LICENSE: MIT (free use for all including commercial)
#
# Problem: "Warning: PoseAsset XXX is out-of-date with its source animation YYY"
#
# Solution: Select the problematic assets (or folders containing the assets)
# in the UnrealEditor Content Browser, then in the UnrealEditor menu bar, select
# `Tools > Execute Python Script...` and select this script.
#
# Expects assets to be named in one of these formats:
#
#   foo_pose.uasset     <=> foo_anim.uasset
#   foo_pose_bar.uasset <=> foo_anim_bar.uasset
#
# (In other words, asset names must end in "_pose" and "_anim",
# or must contain "_pose_" and "_anim_").
#
# To set your project up to allow Python scripting:
# @see https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python
#

import unreal
import re

def update_pose(pose_asset:object, verbose:bool):
    pose_path = pose_asset.get_path_name()

    # Extract just the directory part of the pose asset path
    asset_dir = re.sub(r"(.*)/[^/]+$", r"\1", pose_path)
    # Extract just the name part of the pose asset path
    pose_name = re.sub(r".*\.([^.]+$)", r"\1", pose_path)

    # Compute the expected name of the matching animation asset
    anim_name = re.sub(r"(.+)_pose(_|$)", r"\1_anim\2", pose_name)
    # Compute the full path to the expected matching animation (in the same dir as the pose asset)
    anim_path = asset_dir + '/' + anim_name

    # Try to load the animation
    anim_asset = unreal.EditorAssetLibrary.load_asset(anim_path)
    if not anim_asset or not isinstance(anim_asset, unreal.AnimSequence):
        unreal.log_error("Could not find animation asset for {} (tried: {})".format(pose_path, anim_path))
        return False

    if verbose: unreal.log('Updating {} using {}'.format(pose_name, anim_name))
    pose_asset.update_pose_from_animation(anim_asset)

    # For whatever reason, in UE 5.5 update_pose_from_animation doesn't mark the pose as dirty,
    # so here we will explicitly resave the assets, we can't tell if they changed or not.
    only_if_is_dirty=False
    if unreal.EditorAssetLibrary.save_loaded_asset(pose_asset, only_if_is_dirty):
        if verbose: unreal.log('Saved ' + pose_path)
    return True

def main(verbose:bool):
    unreal.log('Collecting PoseAsset assets from Content Browser selection...')

    # Initialize poseAssets with any explicitly selected ones (array of asset objects)
    pose_assets = unreal.EditorUtilityLibrary.get_selected_assets_of_class(unreal.PoseAsset)

    # If specific folders are selected, recursively scan those for PoseAsset assets
    selected_folders = unreal.EditorUtilityLibrary.get_selected_folder_paths() \
        or unreal.EditorUtilityLibrary.get_selected_path_view_folder_paths()
    for asset_dir in selected_folders:
        asset_dir = asset_dir.removeprefix('/All')  # Is there an Unreal way to do this? :shrug:
        if not unreal.EditorAssetLibrary.does_directory_exist(asset_dir):
            unreal.log_warning('Asset folder does not exist: {}'.format(asset_dir))
            continue
        if verbose: unreal.log('Scanning folder: {}'.format(asset_dir))
        # Recursively list assets under this path
        for asset_path in unreal.EditorAssetLibrary.list_assets(asset_dir, True):
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if asset and isinstance(asset, unreal.PoseAsset):
                pose_assets.append(asset)

    # Print a report of what we found
    if verbose:
        unreal.log('Found {} pose assets:'.format(len(pose_assets)))
        for asset in pose_assets:
            unreal.log('  - ' + asset.get_name())

    # Update PoseAsset as needed
    num_errors = 0
    if len(pose_assets) > 0:
        with unreal.ScopedEditorTransaction("Update {} Poses".format(len(pose_assets))) as trans:
            with unreal.ScopedSlowTask(len(pose_assets), 'Updating Poses...') as slow_task:
                for asset in pose_assets:
                    if slow_task.should_cancel(): break
                    slow_task.enter_progress_frame(1)
                    if not update_pose(asset, verbose):
                        num_errors += 1

    # Output a summary of what we did and whether there were errors
    if num_errors > 0:
        unreal.log_warning('Encountered {} total errors, check log for details.'.format(num_errors))
    unreal.log('Updated {} pose assets with {} errors.'.format(len(pose_assets), num_errors))

if __name__ == "__main__":
    main(True)
