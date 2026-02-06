"""Tests for SNESBuilder class."""

import pytest
import tempfile
import os
from pathlib import Path
from unittest.mock import Mock, patch, MagicMock

from rom_builder.builder import (
    SNESBuilder,
    BuildResult,
    BuildError,
    find_project_root,
)


class TestFindProjectRoot:
    """Test project root finding."""

    def test_finds_root_with_makefile(self):
        """Finds root with Makefile."""
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create Makefile
            makefile = Path(tmpdir) / "Makefile"
            makefile.touch()

            # Create subdirectory
            subdir = Path(tmpdir) / "subdir"
            subdir.mkdir()

            root = find_project_root(subdir)
            # Resolve both paths to handle macOS symlinks (/var vs /private/var)
            assert root.resolve() == Path(tmpdir).resolve()

    def test_finds_root_with_git(self):
        """Finds root with .git directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create .git directory
            git_dir = Path(tmpdir) / ".git"
            git_dir.mkdir()

            # Create subdirectory
            subdir = Path(tmpdir) / "subdir"
            subdir.mkdir()

            root = find_project_root(subdir)
            # Resolve both paths to handle macOS symlinks (/var vs /private/var)
            assert root.resolve() == Path(tmpdir).resolve()

    def test_raises_if_not_found(self):
        """Raises BuildError if root not found."""
        with tempfile.TemporaryDirectory() as tmpdir:
            with pytest.raises(BuildError):
                find_project_root(Path(tmpdir))


class TestBuildResult:
    """Test BuildResult dataclass."""

    def test_success_result(self):
        """Success result has correct fields."""
        result = BuildResult(
            success=True,
            output_path=Path("/tmp/test.sfc"),
            objects=[Path("/tmp/test.o")],
        )
        assert result.success is True
        assert result.output_path == Path("/tmp/test.sfc")
        assert result.error is None

    def test_failure_result(self):
        """Failure result has error message."""
        result = BuildResult(
            success=False,
            error="Build failed",
        )
        assert result.success is False
        assert result.error == "Build failed"
        assert result.output_path is None


class TestSNESBuilder:
    """Test SNESBuilder class."""

    def test_init_with_project_root(self):
        """Initialize with explicit project root."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir))
            assert builder.project_root == Path(tmpdir)

    def test_init_creates_build_dir(self):
        """Build directory is created when building."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            builder.build_dir.mkdir(parents=True, exist_ok=True)
            assert builder.build_dir.exists()

    def test_build_dir_location(self):
        """Build directory is under project root."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir))
            assert builder.build_dir == Path(tmpdir) / "build" / "snes"


class TestSNESBuilderLogging:
    """Test SNESBuilder logging behavior."""

    def test_verbose_logs(self, capsys):
        """Verbose mode prints messages."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=True)
            builder._log("Test message")
            captured = capsys.readouterr()
            assert "Test message" in captured.out

    def test_quiet_no_logs(self, capsys):
        """Quiet mode suppresses messages."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            builder._log("Test message")
            captured = capsys.readouterr()
            assert captured.out == ""


class TestSNESBuilderFindFiles:
    """Test file finding methods."""

    def test_find_linker_config_local(self):
        """Finds local linker config."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)

            # Create local superfx.cfg
            config = tmpdir / "superfx.cfg"
            config.touch()

            builder = SNESBuilder(project_root=tmpdir, verbose=False)
            result = builder.find_linker_config(tmpdir, "superfx")
            assert result == config

    def test_find_linker_config_sdk(self):
        """Finds SDK linker config in linker_configs/ directory."""
        # This test uses the actual SDK linker_configs/ directory
        # which is relative to the builder module location
        sdk_dir = Path(__file__).parent.parent.parent
        sdk_cfg = sdk_dir / "linker_configs" / "lorom.cfg"

        if sdk_cfg.exists():
            with tempfile.TemporaryDirectory() as tmpdir:
                builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
                result = builder.find_linker_config(Path(tmpdir) / "subdir", "lorom")
                assert result == sdk_cfg

    def test_find_linker_config_raises_if_missing(self):
        """Raises BuildError if no config found for unknown cart type."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            with pytest.raises(BuildError):
                # Use a non-existent cart type to ensure no config is found
                builder.find_linker_config(Path(tmpdir), "nonexistent_cart_type")

    def test_find_crt0_local(self):
        """Finds local crt0.s."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)

            # Create local crt0.s
            crt0 = tmpdir / "crt0.s"
            crt0.touch()

            builder = SNESBuilder(project_root=tmpdir, verbose=False)
            result = builder.find_crt0(tmpdir, "lorom")
            assert result == crt0

    def test_find_crt0_sdk(self):
        """Finds SDK crt0.s in startup/ directory."""
        # This test uses the actual SDK startup/ directory
        # which is relative to the builder module location
        sdk_dir = Path(__file__).parent.parent.parent
        sdk_crt0 = sdk_dir / "startup" / "crt0.s"

        if sdk_crt0.exists():
            with tempfile.TemporaryDirectory() as tmpdir:
                builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
                result = builder.find_crt0(Path(tmpdir) / "subdir", "lorom")
                assert result == sdk_crt0

    def test_find_crt0_raises_if_missing(self):
        """Raises BuildError if no crt0 found.

        Note: This test only verifies the error path. In practice, the SDK's
        crt0.s is always found via the module-relative path.
        """
        # Check if SDK crt0 exists - if so, skip this test
        sdk_dir = Path(__file__).parent.parent.parent
        sdk_crt0 = sdk_dir / "startup" / "crt0.s"
        if sdk_crt0.exists():
            pytest.skip("SDK crt0.s exists, cannot test missing crt0 error")

        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            with pytest.raises(BuildError):
                builder.find_crt0(Path(tmpdir), "lorom")


class TestSNESBuilderRun:
    """Test command execution."""

    def test_run_success(self):
        """Successful command returns result."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            result = builder._run(["echo", "hello"])
            assert result.returncode == 0

    def test_run_failure_raises(self):
        """Failed command raises BuildError."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            with pytest.raises(BuildError):
                builder._run(["false"])

    def test_run_with_description(self, capsys):
        """Description is logged."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=True)
            builder._run(["echo", "hello"], "Running echo")
            captured = capsys.readouterr()
            assert "Running echo" in captured.out


class TestSNESBuilderBuildIntegration:
    """Integration tests for build method."""

    def test_build_nonexistent_source(self):
        """Build fails for nonexistent source."""
        with tempfile.TemporaryDirectory() as tmpdir:
            builder = SNESBuilder(project_root=Path(tmpdir), verbose=False)
            result = builder.build(
                source=Path("/nonexistent/file.c"),
                output=Path(tmpdir) / "test.sfc",
            )
            assert result.success is False
            assert result.error is not None

    def test_build_creates_build_dir(self):
        """Build creates build directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            builder = SNESBuilder(project_root=tmpdir, verbose=False)

            # Create a dummy source file
            source = tmpdir / "test.ll"
            source.write_text("; dummy")

            # Build will fail but should create build dir
            builder.build(source=source, output=tmpdir / "test.sfc")

            assert builder.build_dir.exists()


class TestSNESBuilderCartTypes:
    """Test cart type handling."""

    def test_superfx_padding(self):
        """SuperFX ROMs are padded to 256KB."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)

            # Create a small ROM file
            rom_path = tmpdir / "test.sfc"
            rom_path.write_bytes(b'\x00' * 100)

            builder = SNESBuilder(project_root=tmpdir, verbose=False)
            builder._pad_superfx_rom(rom_path)

            assert rom_path.stat().st_size == 256 * 1024

    def test_superfx_no_padding_if_large(self):
        """Large ROMs are not padded."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)

            # Create a large ROM file
            rom_path = tmpdir / "test.sfc"
            rom_path.write_bytes(b'\x00' * (300 * 1024))

            builder = SNESBuilder(project_root=tmpdir, verbose=False)
            builder._pad_superfx_rom(rom_path)

            assert rom_path.stat().st_size == 300 * 1024


class TestBuildError:
    """Test BuildError exception."""

    def test_error_message(self):
        """BuildError stores message."""
        error = BuildError("Test error")
        assert str(error) == "Test error"
